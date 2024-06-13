#include "IDListLayer.hpp"

#include <Geode/modify/MenuLayer.hpp>
class $modify(IDMenuLayer, MenuLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        if (IDListLayer::AREDL_TRIED_LOADING) return true;
        IDListLayer::AREDL_TRIED_LOADING = true;
        IDListLayer::loadAREDL(std::move(m_fields->m_listener), true);

        return true;
    }
};

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(IDLevelSearchLayer, LevelSearchLayer) {
    bool init(int searchType) {
        if (!LevelSearchLayer::init(searchType)) return false;

        auto demonlistButtonSprite = CircleButtonSprite::createWithSpriteFrameName("diffIcon_10_btn_001.png", 1.0f, CircleBaseColor::Pink, CircleBaseSize::Medium);
        demonlistButtonSprite->setScale(0.8f);
        auto demonlistButton = CCMenuItemExt::createSpriteExtra(demonlistButtonSprite, [](auto) {
            CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, IDListLayer::scene()));
        });
        demonlistButton->setID("demonlist-button"_spr);
        auto menu = getChildByID("other-filter-menu");
        menu->addChild(demonlistButton);
        menu->updateLayout();

        return true;
    }
};

// Thanks Cvolton for the code
// https://github.com/Cvolton/betterinfo-geode/blob/v4.0.0/src/hooks/LevelCell.cpp#L113
#include <Geode/modify/LevelCell.hpp>
class $modify(IDLevelCell, LevelCell) {
    void onClick(CCObject* sender) {
        LevelCell::onClick(sender);

        auto layer = m_tableView->getParent()->getParent()->getParent();
        if (layer->getID() == "IDListLayer") static_cast<IDListLayer*>(layer)->deselectKeyboard();
    }
    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (Mod::get()->getSettingValue<bool>("enable-rank")) {
            auto begin = IDListLayer::AREDL.begin();
            auto end = IDListLayer::AREDL.end();
            auto found = std::find(begin, end, m_level->m_levelID.value());
            if (found != end) {
                auto rankTextNode = CCLabelBMFont::create(fmt::format("#{} on AREDL", IDListLayer::AREDL_POSITIONS[found - begin]).c_str(), "chatFont.fnt");
                rankTextNode->setPosition(346.0f, m_level->m_dailyID.value() > 0 ? 6.0f : m_compactView ? 9.0f : 12.0f);
                rankTextNode->setAnchorPoint({ 1.0f, 1.0f });
                rankTextNode->setScale(m_compactView ? 0.45f : 0.6f);
                if (m_level->m_dailyID.value() > 0 || Mod::get()->getSettingValue<bool>("white-rank")) {
                    rankTextNode->setColor({ 255, 255, 255 });
                    rankTextNode->setOpacity(200);
                }
                else {
                    rankTextNode->setColor({ 51, 51, 51 });
                    rankTextNode->setOpacity(152);
                }
                rankTextNode->setID("level-rank-label"_spr);
                m_mainLayer->addChild(rankTextNode);
            }
        }
    }
};

#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(IDKeyboardDispatcher, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat) {
        auto layer = CCDirector::sharedDirector()->getRunningScene()->getChildByID("IDListLayer");
        if (key == KEY_Enter && down && layer) {
            static_cast<IDListLayer*>(layer)->search();
            return true;
        }
        else return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
    }
};
