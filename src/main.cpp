#include <Geode/Geode.hpp>
#include <Geode/modify/CharacterColorPage.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

// The Popup with the search bar
class SearchMenuPopup : public Popup<std::string const&> {
protected:
    bool setup(std::string const& value) override {
        this->setTitle("Workshop Search");

        // Create search bar
        auto searchBar = TextInput::create(200.f, "Search icons...");
        searchBar->setPosition(m_mainLayer->getContentSize() / 2);
        m_mainLayer->addChild(searchBar);

        // Handle search logic here
        searchBar->setCallback([](auto const& text) {
            log::info("User typed: {}", text);
        });

        return true;
    }

public:
    static SearchMenuPopup* create() {
        auto ret = new SearchMenuPopup();
        // Adjust width/height as needed
        if (ret && ret->initAnchored(250.f, 180.f, "")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Hooking the Icon Kit
class $modify(MyIconKit, CharacterColorPage) {
    bool init() {
        if (!CharacterColorPage::init()) return false;

        // "buttons-menu" is the standard ID for the side buttons in Icon Kit
        auto menu = this->getChildByID("buttons-menu");
        if (!menu) return true; 

        // Load your custom sprite using the _spr literal
        // NOTE: workshop-icon.png must be in your mod's 'resources' folder
        auto btnSpr = CCSprite::create("workshop-icon.png"_spr);
        
        // Scale it if it's too big/small
        if (btnSpr) {
            btnSpr->setScale(0.8f);
        }

        auto myBtn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            menu_selector(MyIconKit::onOpenSearch)
        );

        menu->addChild(myBtn);
        myBtn->setID("workshop-search-button"_spr);
        
        // Refresh the layout so it aligns with other buttons
        menu->updateLayout();

        return true;
    }

    void onOpenSearch(CCObject*) {
        SearchMenuPopup::create()->show();
    }
};
