#include "IntegratedDemonlist.hpp"

void IntegratedDemonlist::initializeDemons(web::WebResponse* res, bool pemonlist) {
    auto& list = pemonlist ? PEMONLIST : AREDL;
    list.clear();
    for (auto const& level : res->json().value().as_array()) {
        list.push_back({
            level["level_id"].as_int(),
            level["name"].as_string(),
            level[pemonlist ? "placement" : "position"].as_int()
        });
    }
}

void IntegratedDemonlist::loadAREDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().get("https://api.aredl.net/api/aredl/levels").map([](web::WebResponse* res) {
        if (res->ok()) initializeDemons(res, false);
        else Notification::create("Failed to load AREDL", NotificationIcon::Error)->show();

        task = std::nullopt;
        return *res;
    });
}

void IntegratedDemonlist::loadAREDL(EventListener<web::WebTask>&& listenerRef, utils::MiniFunction<void()> callback) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                initializeDemons(res, false);
                callback();
            }
            else FLAlertLayer::create("Load Failed", "Failed to load AREDL. Please try again later.", "OK")->show();
        }
    });

    listener.setFilter(web::WebRequest().get("https://api.aredl.net/api/aredl/levels"));
}

void IntegratedDemonlist::loadPemonlist() {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().get("https://pemonlist.com/api/list").map([](web::WebResponse* res) {
        if (res->ok()) initializeDemons(res, true);
        else Notification::create("Failed to load Pemonlist", NotificationIcon::Error)->show();

        task = std::nullopt;
        return *res;
    });
}

void IntegratedDemonlist::loadPemonlist(EventListener<web::WebTask>&& listenerRef, utils::MiniFunction<void()> callback) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                initializeDemons(res, true);
                callback();
            }
            else FLAlertLayer::create("Load Failed", "Failed to load Pemonlist. Please try again later.", "OK")->show();
        }
    });

    listener.setFilter(web::WebRequest().get("https://pemonlist.com/api/list"));
}
