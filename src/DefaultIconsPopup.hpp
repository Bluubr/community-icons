#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include "GamemodeViewPopup.hpp"
#include "ModerationPopup.hpp"

using namespace geode::prelude;

struct GamemodeEntry {
    std::string name;
    IconType    type;
};

class DefaultIconsPopup : public geode::Popup {
protected:
    CCMenuItemSpriteExtra*               m_modBtn         = nullptr;
    std::optional<arc::TaskHandle<void>> m_modCheckHandle;

    static std::vector<GamemodeEntry> const& getGamemodes() {
        static const std::vector<GamemodeEntry> s_gamemodes = {
            {"Cube",    IconType::Cube},
            {"Ship",    IconType::Ship},
            {"Ball",    IconType::Ball},
            {"UFO",     IconType::Ufo},
            {"Wave",    IconType::Wave},
            {"Robot",   IconType::Robot},
            {"Spider",  IconType::Spider},
            {"Swing",   IconType::Swing},
            {"Jetpack", IconType::Jetpack},
        };
        return s_gamemodes;
    }

    bool init() {
        if (!Popup::init(380.f, 310.f)) return false;

        this->setTitle("Community Icons");

        m_bgSprite->setColor({35, 35, 50});

        auto winSize = m_mainLayer->getContentSize();
        auto scrollLayer = ScrollLayer::create({winSize.width - 20.f, winSize.height - 50.f});
        scrollLayer->setPosition({10.f, 10.f});
        m_mainLayer->addChild(scrollLayer);

        auto const& gamemodes = getGamemodes();
        float entryH = 56.f;
        float padding = 5.f;
        float totalH = static_cast<float>(gamemodes.size()) * (entryH + padding);
        scrollLayer->m_contentLayer->setContentSize({winSize.width - 20.f, totalH});

        for (size_t i = 0; i < gamemodes.size(); i++) {
            auto const& gm = gamemodes[i];
            float y = totalH - (i * (entryH + padding)) - entryH / 2.f;

            auto bg = CCScale9Sprite::create("GJ_square02.png");
            bg->setContentSize({winSize.width - 28.f, entryH - 4.f});
            bg->setPosition({(winSize.width - 20.f) / 2.f, y});
            bg->setColor({55, 55, 75});
            bg->setOpacity(210);
            scrollLayer->m_contentLayer->addChild(bg);

            auto nameLabel = CCLabelBMFont::create(gm.name.c_str(), "goldFont.fnt");
            nameLabel->setScale(0.6f);
            nameLabel->setAnchorPoint({0.f, 0.5f});
            nameLabel->setPosition({18.f, y});
            scrollLayer->m_contentLayer->addChild(nameLabel);

            auto viewBtnSpr = ButtonSprite::create("View");
            viewBtnSpr->setScale(0.7f);
            auto menu = CCMenu::create();
            auto viewBtn = CCMenuItemSpriteExtra::create(viewBtnSpr, this, menu_selector(DefaultIconsPopup::onViewGamemode));
            viewBtn->setTag(static_cast<int>(i));
            menu->addChild(viewBtn);
            menu->setPosition({winSize.width - 58.f, y});
            scrollLayer->m_contentLayer->addChild(menu);
        }
        scrollLayer->moveToTop();

        auto modMenu = CCMenu::create();
        modMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(modMenu, 10);

        auto badgeSpr = CCSprite::create(
            (Mod::get()->getResourcesDir() / "icon_mod_badge.png").string().c_str());
        if (!badgeSpr) {
            badgeSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        badgeSpr->setScale(0.55f);

        m_modBtn = CCMenuItemSpriteExtra::create(
            badgeSpr, this, menu_selector(DefaultIconsPopup::onOpenModPanel));
        m_modBtn->setPosition({winSize.width - 18.f, winSize.height - 14.f});
        m_modBtn->setVisible(false);
        modMenu->addChild(m_modBtn);

        this->checkModeratorStatus();
        return true;
    }

    void checkModeratorStatus() {
        auto projectId = Mod::get()->getSettingValue<std::string>("firebase-project-id");
        if (projectId.empty()) return;

        int accountID = GJAccountManager::sharedState()->m_accountID;
        if (accountID <= 0) return;

        std::string url =
            "https://firestore.googleapis.com/v1/projects/" + projectId +
            "/databases/(default)/documents/moderators/" +
            std::to_string(accountID);

        auto apiKey = Mod::get()->getSettingValue<std::string>("firebase-api-key");
        if (!apiKey.empty()) url += "?key=" + apiKey;

        Ref<DefaultIconsPopup> selfRef(this);
        m_modCheckHandle = geode::async::spawn(
            web::WebRequest().get(url),
            [selfRef](web::WebResponse response) mutable {
                if (!selfRef || !selfRef->m_modBtn) return;
                if (response.ok()) {
                    selfRef->m_modBtn->setVisible(true);
                }
            });
    }

    void onViewGamemode(CCObject* sender) {
        int idx = static_cast<CCNode*>(sender)->getTag();
        auto const& gm = getGamemodes().at(idx);
        GamemodeViewPopup::create(gm.type, gm.name)->show();
    }

    void onOpenModPanel(CCObject*) {
        ModerationPopup::create()->show();
    }

public:
    static DefaultIconsPopup* create() {
        auto ret = new DefaultIconsPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
