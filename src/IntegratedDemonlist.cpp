#include "IntegratedDemonlist.hpp"

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

void IntegratedDemonlist::isOk(std::string const& url, utils::MiniFunction<void(bool, int)> callback) {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().send("HEAD", url).map([callback](web::WebResponse* res) {
        callback(res->ok(), res->code());
        task = std::nullopt;
        return *res;
    });
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
    isOk("https://api.aredl.net/api/aredl/levels", [](bool ok, int code) {
        if (!ok) {
            log::error("Failed to load AREDL with status code {}", code);
            return;
        }

        task = web::WebRequest().get("https://api.aredl.net/api/aredl/levels").map([](web::WebResponse* res) {
            if (res->ok()) initializeDemons(res, false);

            task = std::nullopt;
            return *res;
        });
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
    isOk("https://api.aredl.net/api/aredl/levels", std::move(okListener), [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get("https://api.aredl.net/api/aredl/levels"));
        else Loader::get()->queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load AREDL. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}

void IntegratedDemonlist::loadPemonlist() {
    static std::optional<web::WebTask> task = std::nullopt;
    isOk("https://pemonlist.com/api/list", [](bool ok, int code) {
        if (!ok) {
            log::error("Failed to load Pemonlist with status code {}", code);
            return;
        }

        task = web::WebRequest().get("https://pemonlist.com/api/list").map([](web::WebResponse* res) {
            if (res->ok()) initializeDemons(res, true);

            task = std::nullopt;
            return *res;
        });
    });
}

void IntegratedDemonlist::loadPemonlist(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, utils::MiniFunction<void()> callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                initializeDemons(res, true);
                callback();
            }
        }
    });

    isOk("https://pemonlist.com/api/list", std::move(okListener), [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get("https://pemonlist.com/api/list"));
        else Loader::get()->queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load Pemonlist. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}
