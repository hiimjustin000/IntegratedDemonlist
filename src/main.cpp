#include "IDListLayer.hpp"

#include <Geode/modify/MenuLayer.hpp>
class $modify(IDMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (IntegratedDemonlist::TRIED_LOADING) return true;
        IntegratedDemonlist::TRIED_LOADING = true;
        IntegratedDemonlist::loadAREDL();
        IntegratedDemonlist::loadPemonlist();

        return true;
    }
};

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

// Thanks Cvolton for the code
// https://github.com/Cvolton/betterinfo-geode/blob/v4.0.0/src/hooks/LevelCell.cpp#L113
#include <Geode/modify/LevelCell.hpp>
class $modify(IDLevelCell, LevelCell) {
    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (Mod::get()->getSettingValue<bool>("enable-rank")) {
            auto rankText = std::string();
            auto found = std::find_if(IntegratedDemonlist::AREDL.begin(), IntegratedDemonlist::AREDL.end(), [this](auto const& demon) {
                return demon.id == m_level->m_levelID;
            });
            if (found != IntegratedDemonlist::AREDL.end()) rankText = fmt::format("#{} AREDL", found->position);
            else {
                found = std::find_if(IntegratedDemonlist::PEMONLIST.begin(), IntegratedDemonlist::PEMONLIST.end(), [this](auto const& demon) {
                    return demon.id == m_level->m_levelID;
                });
                if (found != IntegratedDemonlist::PEMONLIST.end()) rankText = fmt::format("#{} Pemonlist", found->position);
            }
            if (!rankText.empty()) {
                auto rankTextNode = CCLabelBMFont::create(rankText.c_str(), "chatFont.fnt");
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
        if (key == KEY_Enter && down) {
            auto listLayer = static_cast<IDListLayer*>(CCDirector::sharedDirector()->getRunningScene()->getChildByID("IDListLayer"));
            if (listLayer) {
                listLayer->search();
                return true;
            }

            auto packLayer = static_cast<IDPackLayer*>(CCDirector::sharedDirector()->getRunningScene()->getChildByID("IDPackLayer"));
            if (packLayer) {
                packLayer->search();
                return true;
            }

            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
        }
        else return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
    }
};
