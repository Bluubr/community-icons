#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/modify/GJGarageLayer.hpp>

using namespace geode::prelude;

// Define our custom Workshop Popup
class IconWorkshopPopup : public geode::Popup<> {
protected:
    bool setup() override {
        auto winSize = CCDirector::get()->getWinSize();
        this->setTitle("Icon Workshop");

        // 1. Create a container for our list
        auto listMenu = CCMenu::create();
        listMenu->setLayout(
            ColumnLayout::create()
                ->setGap(10.f)
                ->setAutoScale(false)
                ->setAxisReverse(true)
        );

        // 2. Define the gamemodes and their default icon IDs
        // We use the internal IconType enum
        std::vector<IconType> types = {
            IconType::Cube, IconType::Ship, IconType::Ball, IconType::Ufo, 
            IconType::Wave, IconType::Robot, IconType::Spider, IconType::Swing
        };

        for (auto type : types) {
            auto row = CCMenu::create();
            row->setContentSize({ 280.f, 40.f });
            row->setLayout(RowLayout::create()->setGap(20.f));

            // Create the icon sprite (Icon ID 1 = default)
            auto icon = GJItemIcon::createBrowserItem(type, 1);
            icon->setScale(0.8f);
            row->addChild(icon);

            // Create the "View" button
            auto viewBtnSprite = ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 0.8f);
            auto viewBtn = CCMenuItemSpriteExtra::create(
                viewBtnSprite,
                this,
                menu_selector(IconWorkshopPopup::onViewIcon)
            );
            viewBtn->setTag(static_cast<int>(type)); // Store the type in the tag
            row->addChild(viewBtn);

            row->updateLayout();
            listMenu->addChild(row);
        }

        listMenu->updateLayout();
        listMenu->setPosition({winSize.width / 2, winSize.height / 2 - 20.f});
        m_mainLayer->addChild(listMenu);

        return true;
    }

    void onViewIcon(CCObject* sender) {
        auto type = static_cast<IconType>(sender->getTag());
        FLAlertLayer::create("Workshop", "Fetching community icons for this mode...", "OK")->show();
    }

public:
    static IconWorkshopPopup* create() {
        auto ret = new IconWorkshopPopup();
        if (ret && ret->init(320.f, 240.f)) { // Width and Height of popup
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
