#include <Geode/Geode.hpp>
#include <Geode/modify/CharacterColorPage.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class SearchMenuPopup : public Popup<std::string const&> {
protected:
    bool setup(std::string const& value) override {
        this->setTitle("Workshop Search");

        auto searchBar = TextInput::create(200.f, "Search...");
        searchBar->setPosition(m_mainLayer->getContentSize() / 2);
        m_mainLayer->addChild(searchBar);

        searchBar->setCallback([](auto const& text) {
            log::info("Searching for: {}", text);
        });

        return true;
    }

public:
    static SearchMenuPopup* create() {
        auto ret = new SearchMenuPopup();
        if (ret && ret->initAnchored(250.f, 180.f, "")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(MyIconKit, CharacterColorPage) {
    bool init() {
        if (!CharacterColorPage::init()) return false;

        auto menu = this->getChildByID("buttons-menu");
        if (!menu) return true; 

        auto btnSpr = CCSprite::create("workshop-icon.png"_spr);
        
        if (btnSpr) {
            btnSpr->setScale(0.8f);
        } else {
            btnSpr = CCSprite::createWithSpriteFrameName("GJ_searchBtn_001.png");
        }

        auto searchBtn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            menu_selector(MyIconKit::onOpenSearchMenu)
        );

        menu->addChild(searchBtn);
        searchBtn->setID("workshop-search-button"_spr);
        
        menu->updateLayout();

        return true;
    }

    void onOpenSearchMenu(CCObject*) {
        SearchMenuPopup::create()->show();
    }
};
