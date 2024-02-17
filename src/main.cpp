#include "IDListLayer.hpp"

#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/MenuLayer.hpp>

class $modify(IDMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        if (IDListLayer::AREDL.empty()) IDListLayer::loadAREDL();
        return true;
    }
};

class $modify(IDLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* searchObject) {
        if (!LevelBrowserLayer::init(searchObject)) return false;
        if (searchObject->m_searchType == SearchType::MapPack) {
            auto menu = CCMenu::create();
            auto demonlistButtonSprite = CCSprite::create("IDDemonlistButton.png"_spr);
            auto y = demonlistButtonSprite->getContentSize().height / 2 + 4;
            menu->setPosition(CCDirector::sharedDirector()->getScreenRight() - y, y);
            menu->setZOrder(2);
            menu->setID("demonlist-menu"_spr);
            auto demonlistButton = CCMenuItemSpriteExtra::create(demonlistButtonSprite, this, menu_selector(IDLevelBrowserLayer::onDemonList));
            demonlistButton->setID("demonlist-button"_spr);
            menu->addChild(demonlistButton);
            this->addChild(menu);
        }
        return true;
    }

    void onDemonList(CCObject*) {
        auto scene = CCScene::create();
        auto layer = IDListLayer::create();
        scene->addChild(layer);
        CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
    }
};

// Thanks Cvolton for the code
// https://github.com/Cvolton/betterinfo-geode/blob/master/src/hooks/LevelCell.cpp#L113
class $modify(IDLevelCell, LevelCell) {
    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        auto found = std::find(IDListLayer::AREDL.begin(), IDListLayer::AREDL.end(), m_level->m_levelID.value());
        if (found != IDListLayer::AREDL.end()) {
            auto rankTextNode = CCLabelBMFont::create(fmt::format("#{} on AREDL", IDListLayer::AREDL_POSITIONS[found - IDListLayer::AREDL.begin()]).c_str(), "chatFont.fnt");
            rankTextNode->setPosition(346.0f, m_compactView ? 9.0f : 12.0f);
            rankTextNode->setAnchorPoint({ 1.0f, 1.0f });
            rankTextNode->setScale(m_compactView ? 0.45f : 0.6f);
            rankTextNode->setColor({ 51, 51, 51 });
            rankTextNode->setOpacity(152);
            rankTextNode->setID("level-rank-label"_spr);
            m_mainLayer->addChild(rankTextNode);
            if (m_level->m_dailyID > 0 || Mod::get()->getSettingValue<bool>("white-rank")) {
                rankTextNode->setColor({ 255, 255, 255 });
                rankTextNode->setOpacity(200);
            }

            if (m_level->m_dailyID > 0) rankTextNode->setPositionY(6);
        }
    }
};
