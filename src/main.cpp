#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include "DefaultIconsPopup.hpp"

using namespace geode::prelude;

class $modify(MyIconKit, GJGarageLayer) {
    bool init() {
        if (!GJGarageLayer::init()) return false;

        auto menu = this->getChildByID("shards-menu");
        if (!menu) return true;

        auto btnSpr = CCSprite::create(
            (Mod::get()->getResourcesDir() / "workshop-icon.png").string().c_str());
        if (!btnSpr) {
            btnSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        }
        btnSpr->setScale(0.8f);

        auto searchBtn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            menu_selector(MyIconKit::onOpenDefaultIcons)
        );

        menu->addChild(searchBtn);
        searchBtn->setID("workshop-search-button"_spr);
        menu->updateLayout();

        return true;
    }

    void onOpenDefaultIcons(CCObject* sender) {
        DefaultIconsPopup::create()->show();
    }
};
