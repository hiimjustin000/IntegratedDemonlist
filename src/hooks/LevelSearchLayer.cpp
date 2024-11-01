#include "../classes/IDListLayer.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(IDLevelSearchLayer, LevelSearchLayer) {
    bool init(int searchType) {
        if (!LevelSearchLayer::init(searchType)) return false;

        auto demonlistButtonSprite = CircleButtonSprite::createWithSprite("ID_demonBtn_001.png"_spr);
        demonlistButtonSprite->getTopNode()->setScale(1.0f);
        demonlistButtonSprite->setScale(0.8f);
        auto demonlistButton = CCMenuItemExt::createSpriteExtra(demonlistButtonSprite, [](auto) {
            CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, IDListLayer::scene()));
        });
        demonlistButton->setID("demonlist-button"_spr);
        auto menu = getChildByID("other-filter-menu");
        menu->addChild(demonlistButton);
        menu->updateLayout();

        return true;
    }
};
