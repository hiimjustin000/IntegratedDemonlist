#include "IntegratedDemonlist.hpp"

#define AREDL_URL "https://api.aredl.net/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/api/aredl/packs"
#define PEMONLIST_UPTIME_URL "https://pemonlist.com/api/uptime?version=2"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=500&version=2"

void IntegratedDemonlist::initializeAREDL(web::WebResponse* res) {
    AREDL.clear();
    auto str = res->string().value();
    std::string error;
    auto json = matjson::parse(str, error).value_or(matjson::Array());
    if (!error.empty()) log::error("Failed to parse AREDL: {}", error);
    if (json.is_array()) for (auto const& level : json.as_array()) {
        if (level.contains("legacy") && level["legacy"].is_bool() && level["legacy"].as_bool()) continue;
        if (!level.contains("level_id") || !level["level_id"].is_number()) continue;
        if (!level.contains("name") || !level["name"].is_string()) continue;
        if (!level.contains("position") || !level["position"].is_number()) continue;

        AREDL.push_back({
            level["level_id"].as_int(),
            level["name"].as_string(),
            level["position"].as_int()
        });
    }
}

void IntegratedDemonlist::initializePemonlist(web::WebResponse* res) {
    PEMONLIST.clear();
    auto str = res->string().value();
    std::string error;
    auto json = matjson::parse(str, error).value_or(matjson::Object { { "data", matjson::Array() } });
    if (!error.empty()) log::error("Failed to parse Pemonlist: {}", error);
    if (json.is_object() && json.contains("data") && json["data"].is_array()) for (auto const& level : json["data"].as_array()) {
        if (!level.contains("level_id") || !level["level_id"].is_number()) continue;
        if (!level.contains("name") || !level["name"].is_string()) continue;
        if (!level.contains("placement") || !level["placement"].is_number()) continue;

        PEMONLIST.push_back({
            level["level_id"].as_int(),
            level["name"].as_string(),
            level["placement"].as_int()
        });
    }
}

void IntegratedDemonlist::isOk(std::string const& url, EventListener<web::WebTask>&& listenerRef, utils::MiniFunction<void(bool, int)> callback) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) callback(res->ok(), res->code());
    });
    listenerRef.setFilter(web::WebRequest().downloadRange({ 0, 0 }).get(url));
}

void IntegratedDemonlist::loadAREDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    static std::optional<web::WebTask> okTask = std::nullopt;
    okTask = web::WebRequest().downloadRange({ 0, 0 }).get(AREDL_URL).map([](web::WebResponse* res) {
        if (!res->ok()) {
            log::error("Failed to load AREDL with status code {}", res->code());
            okTask = std::nullopt;
            return *res;
        }

        task = web::WebRequest().get(AREDL_URL).map([](web::WebResponse* res2) {
            if (res2->ok()) initializeAREDL(res2);
            else log::error("Failed to load AREDL with status code {}", res2->code());

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
                initializeAREDL(res);
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
    static std::optional<web::WebTask> okTask = std::nullopt;
    okTask = web::WebRequest().downloadRange({ 0, 0 }).get(PEMONLIST_UPTIME_URL).map([](web::WebResponse* res) {
        if (!res->ok()) {
            log::error("Failed to load Pemonlist with status code {}", res->code());
            okTask = std::nullopt;
            return *res;
        }

        task = web::WebRequest().get(PEMONLIST_URL).map([](web::WebResponse* res2) {
            if (res2->ok()) initializePemonlist(res2);
            else log::error("Failed to load Pemonlist with status code {}", res2->code());

            task = std::nullopt;
            return *res2;
        });

        okTask = std::nullopt;
        return *res;
    });
}

void IntegratedDemonlist::loadPemonlist(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, utils::MiniFunction<void()> callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                initializePemonlist(res);
                callback();
            }
            else Loader::get()->queueInMainThread([circle, res] {
                FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load Pemonlist. Please try again later.", "OK")->show();
                circle->setVisible(false);
            });
        }
    });

    isOk(PEMONLIST_UPTIME_URL, std::move(okListener), [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(PEMONLIST_URL));
        else Loader::get()->queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load Pemonlist. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}
