#pragma once

#include <Geode/Geode.hpp>
#include "GamemodeViewPopup.hpp"

using namespace geode::prelude;

struct GamemodeEntry {
    std::string name;
    IconType    type;
};

class DefaultIconsPopup : public Popup<> {
protected:
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

    bool setup() override {
        this->setTitle("Default Icons");

        auto winSize = m_mainLayer->getContentSize();
        float scrollW = winSize.width - 20.f;
        float scrollH = winSize.height - 50.f;

        auto scrollLayer = ScrollLayer::create({scrollW, scrollH});
        scrollLayer->setPosition({10.f, 10.f});
        m_mainLayer->addChild(scrollLayer);

        auto& gamemodes = getGamemodes();
        int   n         = static_cast<int>(gamemodes.size());
        float entryH    = 88.f;
        float padding   = 6.f;
        float totalH    = static_cast<float>(n) * entryH + static_cast<float>(n - 1) * padding;

        scrollLayer->m_contentLayer->setContentSize({scrollW, totalH});

        for (int i = 0; i < n; i++) {
            auto& gm = gamemodes[static_cast<size_t>(i)];
            float y  = totalH - static_cast<float>(i) * (entryH + padding) - entryH / 2.f;

            // Row background
            auto bg = CCScale9Sprite::create("GJ_square02.png");
            bg->setContentSize({scrollW - 8.f, entryH - 6.f});
            bg->setPosition({scrollW / 2.f, y});
            bg->setOpacity(100);
            scrollLayer->m_contentLayer->addChild(bg);

            // Default icon (frame 1) on the left
            auto player = SimplePlayer::create(0);
            player->updatePlayerFrame(1, gm.type);
            player->setScale(0.75f);
            player->setPosition({38.f, y});
            scrollLayer->m_contentLayer->addChild(player);

            // Gamemode name
            auto nameLabel = CCLabelBMFont::create(gm.name.c_str(), "goldFont.fnt");
            nameLabel->setScale(0.65f);
            nameLabel->setAnchorPoint({0.f, 0.5f});
            nameLabel->setPosition({72.f, y});
            scrollLayer->m_contentLayer->addChild(nameLabel);

            // "View" button on the right
            auto viewBtnSpr = ButtonSprite::create("View");
            viewBtnSpr->setScale(0.75f);

            auto menu    = CCMenu::create();
            auto viewBtn = CCMenuItemSpriteExtra::create(
                viewBtnSpr,
                this,
                menu_selector(DefaultIconsPopup::onViewGamemode)
            );
            viewBtn->setTag(i);
            menu->addChild(viewBtn);
            menu->setPosition({scrollW - 45.f, y});
            scrollLayer->m_contentLayer->addChild(menu);
        }

        scrollLayer->moveToTop();

        return true;
    }

    void onViewGamemode(CCObject* sender) {
        auto& gamemodes = getGamemodes();
        int idx = static_cast<CCNode*>(sender)->getTag();
        if (idx >= 0 && idx < static_cast<int>(gamemodes.size())) {
            auto& gm = gamemodes[static_cast<size_t>(idx)];
            GamemodeViewPopup::create(gm.type, gm.name)->show();
        }
    }

public:
    static DefaultIconsPopup* create() {
        auto ret = new DefaultIconsPopup();
        if (ret && ret->initAnchored(380.f, 310.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
