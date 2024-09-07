#include "IntegratedDemonlist.hpp"

#define AREDL_URL "https://api.aredl.net/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/api/aredl/packs"
#define PEMONLIST_URL "https://pemonlist.com/api/list"

void IntegratedDemonlist::initializeDemons(web::WebResponse* res, bool pemonlist) {
    auto& list = pemonlist ? PEMONLIST : AREDL;
    list.clear();
    for (auto const& level : res->json().value().as_array()) {
        if (pemonlist || ((!level.contains("legacy") || !level["legacy"].as_bool()) && !level["two_player"].as_bool())) list.push_back({
            level["level_id"].as_int(),
            level["name"].as_string(),
            level[pemonlist ? "placement" : "position"].as_int()
        });
    }
}

void IntegratedDemonlist::isOk(std::string const& url, EventListener<web::WebTask>&& listenerRef, utils::MiniFunction<void(bool, int)> callback) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) callback(res->ok(), res->code());
    });
    listenerRef.setFilter(web::WebRequest().send("HEAD", url));
}

void IntegratedDemonlist::loadAREDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    static std::optional<web::WebTask> okTask = std::nullopt;
    okTask = web::WebRequest().send("HEAD", AREDL_URL).map([](web::WebResponse* res) {
        if (!res->ok()) {
            log::error("Failed to load AREDL with status code {}", res->code());
            return *res;
        }

        task = web::WebRequest().get(AREDL_URL).map([](web::WebResponse* res2) {
            if (res2->ok()) initializeDemons(res2, false);

            task = std::nullopt;
            return *res2;
        });

        okTask = std::nullopt;
        return *res;
    });
}

void IntegratedDemonlist::loadAREDL(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, utils::MiniFunction<void()> callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                initializeDemons(res, false);
                callback();
            }
        }
    });
    isOk(AREDL_URL, std::move(okListener), [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(AREDL_URL));
        else Loader::get()->queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load AREDL. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}

void IntegratedDemonlist::loadAREDLPacks(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, utils::MiniFunction<void()> callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                AREDL_PACKS.clear();
                for (auto const& pack : res->json().value().as_array()) {
                    std::vector<int> levels;
                    for (auto const& level : pack["levels"].as_array()) levels.push_back(level["level_id"].as_int());
                    AREDL_PACKS.push_back({
                        pack["name"].as_string(),
                        pack["points"].as_double(),
                        levels
                    });
                }
                std::sort(AREDL_PACKS.begin(), AREDL_PACKS.end(), [](auto const& a, auto const& b) {
                    return a.points < b.points;
                });
                callback();
            }
        }
    });
    isOk(AREDL_PACKS_URL, std::move(okListener), [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(AREDL_PACKS_URL));
        else Loader::get()->queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load AREDL packs. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}

void IntegratedDemonlist::loadPemonlist() {
    static std::optional<web::WebTask> task = std::nullopt;
    /*static std::optional<web::WebTask> okTask = std::nullopt;
    okTask = web::WebRequest().send("HEAD", PEMONLIST_URL).map([](web::WebResponse* res) {
        if (!res->ok()) {
            log::error("Failed to load Pemonlist with status code {}", res->code());
            return *res;
        }*/

        task = web::WebRequest().get(PEMONLIST_URL).map([](web::WebResponse* res2) {
            if (res2->ok()) initializeDemons(res2, true);
            else log::error("Failed to load Pemonlist with status code {}", res2->code());

            task = std::nullopt;
            return *res2;
        });

        /*okTask = std::nullopt;
        return *res;
    });*/
}

void IntegratedDemonlist::loadPemonlist(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, utils::MiniFunction<void()> callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                initializeDemons(res, true);
                callback();
            }
            else Loader::get()->queueInMainThread([circle, res] {
                FLAlertLayer::create("Load Failed ({})", fmt::format("Failed to load Pemonlist. Please try again later.", res->code()).c_str(), "OK")->show();
                circle->setVisible(false);
            });
        }
    });

    /*isOk(PEMONLIST_URL, std::move(okListener), [&listener, circle](bool ok, int code) {
        if (ok) */listener.setFilter(web::WebRequest().get(PEMONLIST_URL));/*
        else Loader::get()->queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load Pemonlist. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });*/
}
