#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <chrono>
#include <functional>
#include <vector>

using namespace geode::prelude;

class FirebaseAuth {
public:
    using TokenCallback = std::function<void(std::string const& idToken)>;

    static void withToken(TokenCallback callback) {
        auto apiKey = Mod::get()->getSettingValue<std::string>("firebase-api-key");
        if (apiKey.empty()) {
            callback("");
            return;
        }

        auto now = std::chrono::steady_clock::now();
        if (!s_idToken.empty() && now < s_tokenExpiry) {
            callback(s_idToken);
            return;
        }

        if (s_isFetching) {
            s_pendingCallbacks.push_back(std::move(callback));
            return;
        }
        s_pendingCallbacks.push_back(std::move(callback));

        s_isFetching = true;
        std::string signInUrl =
            "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" + apiKey;

        s_authHandle = geode::async::spawn(
            web::WebRequest()
                .bodyString("{\"returnSecureToken\":true}")
                .header("Content-Type", "application/json")
                .post(signInUrl),
            [](web::WebResponse response) {
                std::string token;
                if (response.ok()) {
                    auto json = response.json();
                    if (json) {
                        auto tokenRes = (*json)["idToken"].asString();
                        if (tokenRes && !(*tokenRes).empty()) {
                            s_idToken     = *tokenRes;
                            s_tokenExpiry = std::chrono::steady_clock::now()
                                          + std::chrono::minutes(50);
                            token = s_idToken;
                        }
                    }
                }
                if (token.empty()) {
                    std::string errMsg;
                    if (response.ok()) {
                        errMsg = "idToken missing from response";
                    } else {
                        auto json = response.json();
                        if (json && (*json).contains("error")) {
                            auto msgRes = (*json)["error"]["message"].asString();
                            if (msgRes) errMsg = *msgRes;
                        }
                        if (errMsg.empty())
                            errMsg = "HTTP " + std::to_string(response.code());
                    }
                    log::warn("FirebaseAuth: anonymous sign-in failed: {}", errMsg);
                }
                s_isFetching = false;
                auto callbacks = std::move(s_pendingCallbacks);
                s_pendingCallbacks.clear();
                for (auto& cb : callbacks) {
                    cb(token);
                }
            });
    }

    static void invalidate() {
        s_idToken.clear();
        s_tokenExpiry = std::chrono::steady_clock::time_point{};
    }

private:
    static inline std::string                           s_idToken;
    static inline std::chrono::steady_clock::time_point s_tokenExpiry;
    static inline bool                                  s_isFetching = false;
    static inline std::vector<TokenCallback>            s_pendingCallbacks;
    static inline std::optional<arc::TaskHandle<void>>  s_authHandle;
};
