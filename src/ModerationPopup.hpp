#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "IconPack.hpp"
#include "FirebaseAuth.hpp"

using namespace geode::prelude;

class ModerationPopup : public geode::Popup {
protected:
    std::vector<IconPack>                m_pendingPacks;
    CCNode*                              m_listContainer = nullptr;
    CCLabelBMFont*                       m_statusLabel   = nullptr;
    std::optional<arc::TaskHandle<void>> m_fetchHandle;
    std::vector<arc::TaskHandle<void>>   m_actionHandles;
    int                                  m_currentPage   = 0;

    static constexpr int ITEMS_PER_PAGE = 5;

    bool init() {
        if (!Popup::init(500.f, 360.f)) return false;

        this->setTitle("Mod Panel");
        m_bgSprite->setColor({30, 20, 45});

        auto winSize = m_mainLayer->getContentSize();

        auto badge = CCSprite::create(
            (Mod::get()->getResourcesDir() / "icon_mod_badge.png").string().c_str());
        if (badge) {
            badge->setScale(0.45f);
            badge->setPosition({winSize.width - 22.f, winSize.height - 13.f});
            m_mainLayer->addChild(badge, 10);
        }

        m_statusLabel = CCLabelBMFont::create("Loading submissions...", "bigFont.fnt");
        m_statusLabel->setScale(0.4f);
        m_statusLabel->setColor({170, 170, 200});
        m_statusLabel->setPosition({winSize.width / 2.f, winSize.height / 2.f});
        m_mainLayer->addChild(m_statusLabel);

        auto navMenu = CCMenu::create();
        navMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(navMenu);

        auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        prevSpr->setScale(0.55f);
        prevSpr->setFlipX(true);
        auto prevBtn = CCMenuItemSpriteExtra::create(
            prevSpr, this, menu_selector(ModerationPopup::onPrevPage));
        prevBtn->setPosition({14.f, winSize.height / 2.f - 14.f});
        navMenu->addChild(prevBtn);

        auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        nextSpr->setScale(0.55f);
        auto nextBtn = CCMenuItemSpriteExtra::create(
            nextSpr, this, menu_selector(ModerationPopup::onNextPage));
        nextBtn->setPosition({winSize.width - 14.f, winSize.height / 2.f - 14.f});
        navMenu->addChild(nextBtn);

        m_listContainer = CCNode::create();
        m_listContainer->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_listContainer);

        this->fetchPending();
        return true;
    }

    static std::string extractDocId(std::string const& docName) {
        auto pos = docName.rfind('/');
        return (pos != std::string::npos) ? docName.substr(pos + 1) : docName;
    }

    static std::string parseStringField(matjson::Value const& f, std::string const& key) {
        if (!f.contains(key)) return "";
        auto fv = f[key];
        if (!fv.contains("stringValue")) return "";
        auto r = fv["stringValue"].asString();
        return r ? *r : "";
    }

    static std::string parseTimestampField(matjson::Value const& f, std::string const& key) {
        if (!f.contains(key)) return "";
        auto fv = f[key];
        if (!fv.contains("timestampValue")) return "";
        auto r = fv["timestampValue"].asString();
        return r ? *r : "";
    }

    static void appendApiKey(std::string& url) {
        auto key = Mod::get()->getSettingValue<std::string>("firebase-api-key");
        if (!key.empty()) {
            url += (url.find('?') == std::string::npos ? "?" : "&");
            url += "key=" + key;
        }
    }

    static std::string buildFirestoreDoc(IconPack const& pack) {
        auto esc = [](std::string const& s) {
            std::string out;
            out.reserve(s.size() + 8);
            for (unsigned char c : s) {
                switch (c) {
                    case '"':  out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\n': out += "\\n";  break;
                    case '\r': out += "\\r";  break;
                    case '\t': out += "\\t";  break;
                    default:
                        if (c < 0x20) {
                            char buf[8];
                            snprintf(buf, sizeof(buf), "\\u%04x", c);
                            out += buf;
                        } else {
                            out += static_cast<char>(c);
                        }
                }
            }
            return out;
        };

        return std::string("{\"fields\":{")
            + "\"name\":{\"stringValue\":\"" + esc(pack.name) + "\"},"
            + "\"author\":{\"stringValue\":\"" + esc(pack.author) + "\"},"
            + "\"gamemode\":{\"stringValue\":\"" + esc(pack.gamemode) + "\"},"
            + "\"imageUrl\":{\"stringValue\":\"" + esc(pack.imageUrl) + "\"},"
            + "\"plistUrl\":{\"stringValue\":\"" + esc(pack.plistUrl) + "\"},"
            + "\"graphicsType\":{\"stringValue\":\"" + esc(pack.graphicsType) + "\"},"
            + "\"Downloads\":{\"integerValue\":\"0\"}"
            + "}}";
    }

    void fetchPending() {
        auto projectId = Mod::get()->getSettingValue<std::string>("firebase-project-id");
        if (projectId.empty()) {
            m_statusLabel->setString("Firebase Project ID not set");
            return;
        }

        std::string url =
            "https://firestore.googleapis.com/v1/projects/" + projectId +
            "/databases/(default)/documents/pendingIconPacks";
        appendApiKey(url);

        Ref<ModerationPopup> selfRef(this);
        m_fetchHandle = geode::async::spawn(
            web::WebRequest().get(url),
            [selfRef](web::WebResponse response) mutable {
                if (!selfRef) return;
                if (!response.ok()) {
                    selfRef->m_statusLabel->setString(
                        ("Fetch error (" + std::to_string(response.code()) + ")").c_str());
                    return;
                }
                auto json = response.json();
                if (!json) {
                    selfRef->m_statusLabel->setString("Parse error");
                    return;
                }

                std::vector<IconPack> packs;
                if ((*json).contains("documents")) {
                    auto docs = (*json)["documents"].asArray();
                    if (docs) {
                        for (auto const& doc : *docs) {
                            if (!doc.contains("name") || !doc.contains("fields")) continue;
                            auto f = doc["fields"];
                            IconPack pack;
                            auto nameRes = doc["name"].asString();
                            pack.id         = extractDocId(nameRes ? *nameRes : "");
                            pack.name       = parseStringField(f, "name");
                            pack.author     = parseStringField(f, "author");
                            pack.gamemode   = parseStringField(f, "gamemode");
                            pack.imageUrl   = parseStringField(f, "imageUrl");
                            pack.plistUrl   = parseStringField(f, "plistUrl");
                            pack.graphicsType = parseStringField(f, "graphicsType");
                            pack.uploadedAt = parseTimestampField(f, "uploadedAt");
                            packs.push_back(std::move(pack));
                        }
                    }
                }
                selfRef->m_pendingPacks = std::move(packs);
                selfRef->m_currentPage  = 0;
                selfRef->refreshList();
            });
    }

    void refreshList() {
        m_listContainer->removeAllChildren();

        if (m_pendingPacks.empty()) {
            m_statusLabel->setString("No pending submissions");
            m_statusLabel->setVisible(true);
            return;
        }
        m_statusLabel->setVisible(false);

        auto winSize   = m_mainLayer->getContentSize();
        int  total     = static_cast<int>(m_pendingPacks.size());
        int  totalPages = std::max(1, (total + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE);
        if (m_currentPage >= totalPages) m_currentPage = totalPages - 1;
        if (m_currentPage < 0)           m_currentPage = 0;

        float rowH   = 52.f;
        float startY = winSize.height - 58.f;

        int startIdx = m_currentPage * ITEMS_PER_PAGE;
        int endIdx   = std::min(startIdx + ITEMS_PER_PAGE, total);

        for (int i = startIdx; i < endIdx; ++i) {
            buildRow(i, startY - (i - startIdx) * rowH, winSize.width);
        }
    }

    void buildRow(int idx, float y, float w) {
        auto const& pack = m_pendingPacks[idx];

        auto bg = CCScale9Sprite::create("GJ_square02.png");
        bg->setContentSize({w - 44.f, 46.f});
        bg->setPosition({w / 2.f, y});
        bg->setColor({50, 40, 65});
        bg->setOpacity(210);
        m_listContainer->addChild(bg);

        auto nameLabel = CCLabelBMFont::create(
            pack.name.empty() ? "Unnamed" : pack.name.c_str(), "chatFont.fnt");
        nameLabel->setScale(0.45f);
        nameLabel->setColor({220, 220, 255});
        nameLabel->limitLabelWidth(w * 0.42f, 0.45f, 0.1f);
        nameLabel->setAnchorPoint({0.f, 0.5f});
        nameLabel->setPosition({24.f, y + 8.f});
        m_listContainer->addChild(nameLabel);

        std::string meta = pack.gamemode;
        if (!pack.author.empty()) meta += " · by " + pack.author;
        auto metaLabel = CCLabelBMFont::create(meta.c_str(), "chatFont.fnt");
        metaLabel->setScale(0.3f);
        metaLabel->setColor({150, 150, 185});
        metaLabel->limitLabelWidth(w * 0.42f, 0.3f, 0.08f);
        metaLabel->setAnchorPoint({0.f, 0.5f});
        metaLabel->setPosition({24.f, y - 9.f});
        m_listContainer->addChild(metaLabel);

        auto actionMenu = CCMenu::create();
        actionMenu->setPosition({0.f, 0.f});
        m_listContainer->addChild(actionMenu);

        auto acceptSpr = ButtonSprite::create("Accept");
        acceptSpr->setScale(0.5f);
        auto acceptBtn = CCMenuItemSpriteExtra::create(
            acceptSpr, this, menu_selector(ModerationPopup::onAccept));
        acceptBtn->setTag(idx);
        acceptBtn->setPosition({w - 94.f, y + 9.f});
        actionMenu->addChild(acceptBtn);

        auto declineSpr = ButtonSprite::create("Decline");
        declineSpr->setScale(0.5f);
        auto declineBtn = CCMenuItemSpriteExtra::create(
            declineSpr, this, menu_selector(ModerationPopup::onDecline));
        declineBtn->setTag(idx);
        declineBtn->setPosition({w - 94.f, y - 11.f});
        actionMenu->addChild(declineBtn);
    }

    void onAccept(CCObject* sender) {
        int idx = static_cast<CCNode*>(sender)->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_pendingPacks.size())) return;
        auto const& pack = m_pendingPacks[idx];

        auto projectId = Mod::get()->getSettingValue<std::string>("firebase-project-id");
        if (projectId.empty()) return;

        std::string postUrl =
            "https://firestore.googleapis.com/v1/projects/" + projectId +
            "/databases/(default)/documents/iconPacks";
        appendApiKey(postUrl);

        std::string deleteUrl =
            "https://firestore.googleapis.com/v1/projects/" + projectId +
            "/databases/(default)/documents/pendingIconPacks/" + pack.id;
        appendApiKey(deleteUrl);

        std::string body    = buildFirestoreDoc(pack);
        std::string packId  = pack.id;
        std::string packName = pack.name.empty() ? "Unnamed" : pack.name;
        Ref<ModerationPopup> selfRef(this);

        FirebaseAuth::withToken([selfRef, postUrl, deleteUrl, body, packId, packName]
                                (std::string const& idToken) mutable {
            if (!selfRef) return;

            auto req = web::WebRequest()
                .bodyString(body)
                .header("Content-Type", "application/json");
            if (!idToken.empty())
                req = req.header("Authorization", "Bearer " + idToken);

            selfRef->m_actionHandles.push_back(geode::async::spawn(
                req.post(postUrl),
                [selfRef, deleteUrl, packId, packName](web::WebResponse response) mutable {
                    if (!selfRef) return;
                    if (!response.ok()) {
                        if (response.code() == 401 || response.code() == 403)
                            FirebaseAuth::invalidate();
                        FLAlertLayer::create(
                            nullptr, "Error",
                            "Failed to accept \"" + packName + "\" (" +
                                std::to_string(response.code()) + ")",
                            "OK", nullptr, 340.f)->show();
                        return;
                    }
                    FirebaseAuth::withToken([selfRef, deleteUrl, packId]
                                           (std::string const& delToken) mutable {
                        if (!selfRef) return;
                        auto delReq = web::WebRequest();
                        if (!delToken.empty())
                            delReq = delReq.header("Authorization", "Bearer " + delToken);
                        selfRef->m_actionHandles.push_back(geode::async::spawn(
                            delReq.send("DELETE", deleteUrl),
                            [selfRef, packId](web::WebResponse /*resp*/) mutable {
                                if (!selfRef) return;
                                selfRef->removePack(packId);
                            }));
                    });
                }));
        });
    }

    void onDecline(CCObject* sender) {
        int idx = static_cast<CCNode*>(sender)->getTag();
        if (idx < 0 || idx >= static_cast<int>(m_pendingPacks.size())) return;

        std::string packId   = m_pendingPacks[idx].id;
        std::string packName = m_pendingPacks[idx].name.empty()
                             ? "Unnamed" : m_pendingPacks[idx].name;
        auto projectId = Mod::get()->getSettingValue<std::string>("firebase-project-id");
        if (projectId.empty()) return;

        std::string deleteUrl =
            "https://firestore.googleapis.com/v1/projects/" + projectId +
            "/databases/(default)/documents/pendingIconPacks/" + packId;
        appendApiKey(deleteUrl);

        Ref<ModerationPopup> selfRef(this);

        FirebaseAuth::withToken([selfRef, deleteUrl, packId, packName]
                                (std::string const& idToken) mutable {
            if (!selfRef) return;

            auto req = web::WebRequest();
            if (!idToken.empty())
                req = req.header("Authorization", "Bearer " + idToken);

            selfRef->m_actionHandles.push_back(geode::async::spawn(
                req.send("DELETE", deleteUrl),
                [selfRef, packId, packName](web::WebResponse response) mutable {
                    if (!selfRef) return;
                    if (!response.ok()) {
                        if (response.code() == 401 || response.code() == 403)
                            FirebaseAuth::invalidate();
                        FLAlertLayer::create(
                            nullptr, "Error",
                            "Failed to decline \"" + packName + "\" (" +
                                std::to_string(response.code()) + ")",
                            "OK", nullptr, 340.f)->show();
                        return;
                    }
                    selfRef->removePack(packId);
                }));
        });
    }

    void removePack(std::string const& packId) {
        m_pendingPacks.erase(
            std::remove_if(m_pendingPacks.begin(), m_pendingPacks.end(),
                [&packId](IconPack const& p) { return p.id == packId; }),
            m_pendingPacks.end());
        refreshList();
    }

    void onPrevPage(CCObject*) {
        if (m_currentPage > 0) { --m_currentPage; refreshList(); }
    }

    void onNextPage(CCObject*) {
        int total = static_cast<int>(m_pendingPacks.size());
        int pages = std::max(1, (total + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE);
        if (m_currentPage < pages - 1) { ++m_currentPage; refreshList(); }
    }

public:
    static ModerationPopup* create() {
        auto ret = new ModerationPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
