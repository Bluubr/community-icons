#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <filesystem>
#include <fstream>
#include "IconPack.hpp"

using namespace geode::prelude;

class IconPackDetailPopup : public geode::Popup {
protected:
    IconPack       m_pack;

    CCSprite*      m_previewSprite = nullptr;
    CCLabelBMFont* m_statusLabel   = nullptr;
    TextInput*     m_iconNameInput = nullptr;

    std::vector<arc::TaskHandle<void>> m_handles;

    bool init(IconPack const& pack) {
        if (!Popup::init(440.f, 360.f)) return false;

        m_pack = pack;

        this->setTitle(pack.name.empty() ? "Icon Pack" : pack.name);
        m_bgSprite->setColor({35, 35, 50});

        auto winSize = m_mainLayer->getContentSize();

        std::string subLine;
        if (!pack.author.empty())       subLine = "by " + pack.author;
        if (!pack.graphicsType.empty()) {
            if (!subLine.empty()) subLine += "  |  ";
            subLine += pack.graphicsType;
        }
        if (!subLine.empty()) {
            auto subLbl = CCLabelBMFont::create(subLine.c_str(), "chatFont.fnt");
            subLbl->setScale(0.38f);
            subLbl->setColor({150, 150, 185});
            subLbl->setPosition({winSize.width / 2.f, winSize.height - 40.f});
            m_mainLayer->addChild(subLbl);
        }

        float previewH = winSize.height * 0.38f;
        float previewY = winSize.height - 58.f - previewH / 2.f;

        auto previewBg = CCScale9Sprite::create("GJ_square02.png");
        previewBg->setContentSize({winSize.width - 40.f, previewH});
        previewBg->setPosition({winSize.width / 2.f, previewY});
        previewBg->setColor({45, 45, 65});
        previewBg->setOpacity(210);
        m_mainLayer->addChild(previewBg);

        m_previewSprite = CCSprite::create();
        m_previewSprite->setPosition({winSize.width / 2.f, previewY});
        m_previewSprite->setVisible(false);
        m_mainLayer->addChild(m_previewSprite);

        float inputRowY = previewY - previewH / 2.f - 26.f;

        auto nameLbl = CCLabelBMFont::create("Replace icon:", "chatFont.fnt");
        nameLbl->setScale(0.42f);
        nameLbl->setColor({200, 200, 225});
        nameLbl->setAnchorPoint({1.f, 0.5f});
        nameLbl->setPosition({winSize.width / 2.f - 4.f, inputRowY});
        m_mainLayer->addChild(nameLbl);

        m_iconNameInput = TextInput::create(150.f, "e.g. Cube_104");
        m_iconNameInput->setPosition({winSize.width / 2.f + 81.f, inputRowY});
        m_mainLayer->addChild(m_iconNameInput);

        std::string suffixNote = "File suffix: ";
        if (pack.graphicsType.empty() || pack.graphicsType == "Standard") {
            suffixNote += "(none)";
        } else {
            suffixNote += pack.graphicsType;
        }
        auto hintLbl = CCLabelBMFont::create(suffixNote.c_str(), "chatFont.fnt");
        hintLbl->setScale(0.28f);
        hintLbl->setColor({110, 110, 145});
        hintLbl->setPosition({winSize.width / 2.f, inputRowY - 14.f});
        m_mainLayer->addChild(hintLbl);

        m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_statusLabel->setScale(0.33f);
        m_statusLabel->setColor({220, 80, 80});
        m_statusLabel->setPosition({winSize.width / 2.f, 36.f});
        m_mainLayer->addChild(m_statusLabel);

        auto btnMenu = CCMenu::create();
        btnMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(btnMenu);

        auto applySpr = ButtonSprite::create("Apply Icon");
        applySpr->setScale(0.75f);
        auto applyBtn = CCMenuItemSpriteExtra::create(
            applySpr, this,
            menu_selector(IconPackDetailPopup::onApply));
        applyBtn->setPosition({winSize.width / 2.f, 16.f});
        btnMenu->addChild(applyBtn);

        if (!pack.imageUrl.empty()) {
            loadPreview(pack.imageUrl, winSize.width - 60.f, previewH - 8.f);
        }

        return true;
    }

    void loadPreview(std::string const& url, float maxW, float maxH) {
        std::string cacheKey = "ci_detail_" + m_pack.id;
        Ref<IconPackDetailPopup> selfRef(this);

        m_handles.push_back(geode::async::spawn(
            web::WebRequest().get(url),
            [selfRef, cacheKey, maxW, maxH](web::WebResponse response) mutable {
                if (!selfRef || !response.ok()) return;

                auto const& bytes = response.data();
                if (bytes.empty()) return;

                auto* tex = CCTextureCache::sharedTextureCache()
                                ->textureForKey(cacheKey.c_str());
                if (!tex) {
                    auto* img = new CCImage();
                    if (img->initWithImageData(
                            const_cast<unsigned char*>(bytes.data()),
                            static_cast<int>(bytes.size()))) {
                        tex = CCTextureCache::sharedTextureCache()
                                  ->addUIImage(img, cacheKey.c_str());
                    }
                    img->release();
                }

                if (!tex || !selfRef->m_previewSprite) return;
                auto sz = tex->getContentSize();
                if (sz.width <= 0.f || sz.height <= 0.f) return;

                selfRef->m_previewSprite->setTexture(tex);
                selfRef->m_previewSprite->setTextureRect(
                    {0.f, 0.f, sz.width, sz.height});
                float scale = std::min(maxW / sz.width, maxH / sz.height);
                selfRef->m_previewSprite->setScale(std::max(scale, 0.01f));
                selfRef->m_previewSprite->setVisible(true);
            }));
    }

    void setStatus(const char* msg, bool error) {
        if (!m_statusLabel) return;
        m_statusLabel->setString(msg);
        m_statusLabel->setColor(
            error ? ccColor3B{220, 80, 80} : ccColor3B{80, 200, 100});
    }

    static constexpr const char* TL_PACK_NAME = "Community Icons";

    static std::string trimWhitespace(std::string s) {
        auto first = s.find_first_not_of(" \t");
        if (first == std::string::npos) return "";
        auto last = s.find_last_not_of(" \t");
        return s.substr(first, last - first + 1);
    }

    static bool ensureCommunityIconsDir(
        std::filesystem::path& outIconsDir, std::string& errOut)
    {
        auto tlConfigDir =
            Mod::get()->getConfigDir().parent_path() / "geode.texture-loader";

        auto iconsDir = tlConfigDir / TL_PACK_NAME / "icons";
        std::error_code ec;
        std::filesystem::create_directories(iconsDir, ec);
        if (ec) {
            errOut = "Could not create folder: " + ec.message();
            return false;
        }
        outIconsDir = iconsDir;
        return true;
    }

    static std::string graphicsSuffix(std::string const& gt) {
        if (gt == "UHD" || gt == "HD") return gt;
        return "";
    }

    static bool isValidIconName(std::string const& name) {
        if (name.empty() || name.size() > 64) return false;
        for (char c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) &&
                c != '_' && c != '-') return false;
        }
        return true;
    }

    static bool isHttpsUrl(std::string const& url) {
        if (url.size() < 8) return false;
        std::string prefix = url.substr(0, 8);
        std::transform(prefix.begin(), prefix.end(), prefix.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return prefix == "https://";
    }

    void onApply(CCObject*) {
        if (m_pack.imageUrl.empty()) {
            setStatus("No image URL for this pack.", true);
            return;
        }

        std::string iconName = trimWhitespace(
            m_iconNameInput ? m_iconNameInput->getString() : "");

        if (iconName.empty()) {
            setStatus("Please enter an icon name (e.g. Cube_104).", true);
            return;
        }

        if (!isValidIconName(iconName)) {
            setStatus("Icon name may only contain letters, digits, _ and -.", true);
            return;
        }

        if (!isHttpsUrl(m_pack.imageUrl)) {
            setStatus("Pack has an invalid image URL.", true);
            return;
        }
        if (!m_pack.plistUrl.empty() && !isHttpsUrl(m_pack.plistUrl)) {
            setStatus("Pack has an invalid plist URL.", true);
            return;
        }

        std::filesystem::path iconsDir;
        std::string dirErr;
        if (!ensureCommunityIconsDir(iconsDir, dirErr)) {
            setStatus(dirErr.c_str(), true);
            return;
        }

        setStatus("Downloading...", false);

        std::string suffix   = graphicsSuffix(m_pack.graphicsType);
        std::string baseName = iconName + suffix;
        std::string imageUrl = m_pack.imageUrl;
        std::string plistUrl = m_pack.plistUrl;
        Ref<IconPackDetailPopup> selfRef(this);

        m_handles.push_back(geode::async::spawn(
            web::WebRequest().get(imageUrl),
            [selfRef, iconsDir, baseName, plistUrl]
            (web::WebResponse imgResp) mutable {
                if (!selfRef) return;
                if (!imgResp.ok()) {
                    selfRef->setStatus("Image download failed.", true);
                    return;
                }
                auto const& imgBytes = imgResp.data();
                if (imgBytes.empty()) {
                    selfRef->setStatus("Empty image response.", true);
                    return;
                }

                auto imgPath = iconsDir / (baseName + ".png");
                {
                    std::ofstream ofs(imgPath, std::ios::binary | std::ios::trunc);
                    if (!ofs) {
                        selfRef->setStatus("Failed to write image file.", true);
                        return;
                    }
                    ofs.write(
                        reinterpret_cast<const char*>(imgBytes.data()),
                        static_cast<std::streamsize>(imgBytes.size()));
                }

                if (plistUrl.empty()) {
                    selfRef->setStatus(
                        ("Saved " + baseName + ".png to Community Icons/icons/").c_str(),
                        false);
                    return;
                }

                selfRef->m_handles.push_back(geode::async::spawn(
                    web::WebRequest().get(plistUrl),
                    [selfRef, iconsDir, baseName]
                    (web::WebResponse plistResp) mutable {
                        if (!selfRef) return;
                        if (!plistResp.ok()) {
                            selfRef->setStatus(
                                "Image saved but plist download failed.", false);
                            return;
                        }
                        auto const& plistBytes = plistResp.data();
                        if (plistBytes.empty()) {
                            selfRef->setStatus(
                                "Image saved but plist was empty.", false);
                            return;
                        }

                        auto plistPath = iconsDir / (baseName + ".plist");
                        {
                            std::ofstream ofs(
                                plistPath, std::ios::binary | std::ios::trunc);
                            if (!ofs) {
                                selfRef->setStatus(
                                    "Image saved but failed to write plist.", false);
                                return;
                            }
                            ofs.write(
                                reinterpret_cast<const char*>(plistBytes.data()),
                                static_cast<std::streamsize>(plistBytes.size()));
                        }

                        selfRef->setStatus(
                            ("Saved " + baseName + " to Community Icons/icons/."
                             " Restart GD to apply.").c_str(),
                            false);
                    }));
            }));
    }

public:
    static IconPackDetailPopup* create(IconPack const& pack) {
        auto ret = new IconPackDetailPopup();
        if (ret && ret->init(pack)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
