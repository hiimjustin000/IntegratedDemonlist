#include "../classes/IDPackLayer.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(IDLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* object) {
        if (!LevelBrowserLayer::init(object)) return false;

        if (object->m_searchType == SearchType::MapPack) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto demonlistButtonSprite = CircleButtonSprite::createWithSprite("ID_demonBtn_001.png"_spr);
            demonlistButtonSprite->getTopNode()->setScale(1.0f);
            auto demonlistButton = CCMenuItemExt::createSpriteExtra(demonlistButtonSprite, [](auto) {
                CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, IDPackLayer::scene()));
            });
            demonlistButton->setID("demonlist-button"_spr);
            auto y = demonlistButtonSprite->getContentHeight() / 2 + 4.0f;
            auto menu = CCMenu::create();
            menu->addChild(demonlistButton);
            menu->setPosition(winSize.width - y, y);
            menu->setID("demonlist-menu"_spr);
            addChild(menu, 2);
        }

        return true;
    }
};
