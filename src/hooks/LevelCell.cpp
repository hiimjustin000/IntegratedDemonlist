#include "../IntegratedDemonlist.hpp"

using namespace geode::prelude;

#define AREDL_LEVEL_URL "https://api.aredl.net/api/aredl/levels/{}"
#define AREDL_LEVEL_2P_URL "https://api.aredl.net/api/aredl/levels/{}?two_player=true"
#define PEMONLIST_LEVEL_URL "https://pemonlist.com/api/level/{}?version=2"

#include <Geode/modify/LevelCell.hpp>
class $modify(IDLevelCell, LevelCell) {
    struct Fields {
        EventListener<web::WebTask> m_soloListener;
        EventListener<web::WebTask> m_dualListener;
    };

    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (m_level->m_demon.value() <= 0 || !Mod::get()->getSettingValue<bool>("enable-rank")) return;

        std::vector<std::string> positions;
        auto platformer = m_level->m_levelLength == 5;
        auto& list = platformer ? IntegratedDemonlist::PEMONLIST : IntegratedDemonlist::AREDL;
        auto levelID = m_level->m_levelID.value();
        for (auto const& demon : list) {
            if (demon.id == levelID) positions.push_back(std::to_string(demon.position));
        }

        if (!positions.empty()) {
            addRank(positions, platformer);
            return;
        }

        auto f = m_fields.self();
        f->m_soloListener.bind([this, f, levelID, platformer](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (!res->ok()) return;
                auto str = res->string().value();
                std::string error;
                auto json = matjson::parse(str, error).value_or(matjson::Object());
                if (!error.empty()) log::error("Failed to parse {} level {}: {}", platformer ? "Pemonlist" : "AREDL", levelID, error);
                auto key = platformer ? "placement" : "position";
                if (!json.is_object() || !json.contains(key) || !json[key].is_number()) return;

                auto position1 = json[key].as_int();
                auto& list = platformer ? IntegratedDemonlist::PEMONLIST : IntegratedDemonlist::AREDL;
                std::string levelName = m_level->m_levelName;
                list.push_back({ levelID, levelName, position1 });
                if (platformer) {
                    addRank({ std::to_string(position1) }, platformer);
                    return;
                }

                f->m_dualListener.bind([this, levelID, levelName, position1](web::WebTask::Event* e) {
                    if (auto res = e->getValue()) {
                        if (!res->ok()) {
                            addRank({ std::to_string(position1) }, false);
                            return;
                        }

                        auto str = res->string().value();
                        std::string error;
                        auto json = matjson::parse(str, error).value_or(matjson::Object());
                        if (!error.empty()) log::error("Failed to parse AREDL two-player level {}: {}", levelID, error);
                        if (!json.is_object() || !json.contains("position") || !json["position"].is_number()) return;

                        auto position2 = json["position"].as_int();
                        IntegratedDemonlist::AREDL.push_back({ levelID, levelName, position2 });
                        addRank({ std::to_string(position1), std::to_string(position2) }, false);
                    }
                });

                f->m_dualListener.setFilter(web::WebRequest().get(fmt::format(AREDL_LEVEL_2P_URL, levelID)));
            }
        });

        f->m_soloListener.setFilter(web::WebRequest().get(platformer ? fmt::format(PEMONLIST_LEVEL_URL, levelID) : fmt::format(AREDL_LEVEL_URL, levelID)));
    }

    void addRank(std::vector<std::string> const& positions, bool platformer) {
        auto rankTextNode = CCLabelBMFont::create(fmt::format("#{} {}", string::join(positions, "/#"), platformer ? "Pemonlist" : "AREDL").c_str(), "chatFont.fnt");
        auto dailyLevel = m_level->m_dailyID.value() > 0;
        rankTextNode->setPosition(346.0f, dailyLevel ? 6.0f : 1.0f);
        rankTextNode->setAnchorPoint({ 1.0f, 0.0f });
        rankTextNode->setScale(m_compactView ? 0.45f : 0.6f);
        auto isWhite = Mod::get()->getSettingValue<bool>("white-rank");
        rankTextNode->setColor(dailyLevel || isWhite ? ccColor3B { 255, 255, 255 } : ccColor3B { 51, 51, 51 });
        rankTextNode->setOpacity(dailyLevel || isWhite ? 200 : 152);
        rankTextNode->setID("level-rank-label"_spr);
        m_mainLayer->addChild(rankTextNode);

        if (auto levelSizeLabel = m_mainLayer->getChildByID("hiimjustin000.level_size/size-label")) levelSizeLabel->setPosition(
            346.0f - (m_compactView ? rankTextNode->getScaledContentWidth() + 3.0f : 0.0f),
            !m_compactView ? 12.0f : 1.0f
        );
    }
};
