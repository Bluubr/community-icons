#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class GamemodeViewPopup : public Popup<IconType, std::string> {
protected:
    IconType m_iconType;
    std::string m_gamemodeName;
    std::string m_searchText;
    ScrollLayer* m_scrollLayer = nullptr;

    bool setup(IconType iconType, std::string name) override {
        m_iconType = iconType;
        m_gamemodeName = name;

        this->setTitle(name);

        auto winSize = m_mainLayer->getContentSize();

        // Search bar at the top
        auto searchBar = TextInput::create(winSize.width - 40.f, "Search by number...");
        searchBar->setPosition({winSize.width / 2, winSize.height - 32.f});
        m_mainLayer->addChild(searchBar);

        searchBar->setCallback([this](std::string const& text) {
            m_searchText = text;
            this->refreshIcons();
        });

        // Scrollable icon grid
        float scrollH = winSize.height - 72.f;
        m_scrollLayer = ScrollLayer::create({winSize.width - 20.f, scrollH});
        m_scrollLayer->setPosition({10.f, 10.f});
        m_mainLayer->addChild(m_scrollLayer);

        this->refreshIcons();

        return true;
    }

    void refreshIcons() {
        m_scrollLayer->m_contentLayer->removeAllChildren();

        int totalCount = GameManager::get()->countForType(m_iconType);
        float iconSize = 48.f;
        float padding  = 8.f;
        int   cols     = 5;

        std::vector<int> filtered;
        filtered.reserve(static_cast<size_t>(totalCount));
        for (int i = 1; i <= totalCount; i++) {
            if (!m_searchText.empty() &&
                std::to_string(i).find(m_searchText) == std::string::npos)
                continue;
            filtered.push_back(i);
        }

        int   rows       = (static_cast<int>(filtered.size()) + cols - 1) / cols;
        float totalH     = std::max(
            m_scrollLayer->getContentSize().height,
            static_cast<float>(rows) * (iconSize + padding) + padding
        );

        m_scrollLayer->m_contentLayer->setContentSize(
            {m_scrollLayer->getContentSize().width, totalH}
        );

        float startX = padding + iconSize / 2.f;
        float startY = totalH - padding - iconSize / 2.f;

        for (int idx = 0; idx < static_cast<int>(filtered.size()); idx++) {
            int frame = filtered[idx];
            int col   = idx % cols;
            int row   = idx / cols;

            float x = startX + static_cast<float>(col) * (iconSize + padding);
            float y = startY - static_cast<float>(row) * (iconSize + padding);

            auto player = SimplePlayer::create(0);
            player->updatePlayerFrame(frame, m_iconType);
            player->setScale(0.55f);
            player->setPosition({x, y});
            m_scrollLayer->m_contentLayer->addChild(player);

            auto numLabel = CCLabelBMFont::create(
                std::to_string(frame).c_str(), "chatFont.fnt"
            );
            numLabel->setScale(0.45f);
            numLabel->setPosition({x, y - iconSize / 2.f + 4.f});
            m_scrollLayer->m_contentLayer->addChild(numLabel);
        }

        m_scrollLayer->moveToTop();
    }

public:
    static GamemodeViewPopup* create(IconType type, std::string const& name) {
        auto ret = new GamemodeViewPopup();
        if (ret && ret->initAnchored(360.f, 290.f, type, name)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
