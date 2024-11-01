#pragma once
#include <Geode/utils/web.hpp>

struct IDListDemon {
    int id;
    std::string name;
    int position;
};

struct IDDemonPack {
    std::string name;
    double points;
    std::vector<int> levels;
};

class IntegratedDemonlist {
public:
    inline static std::vector<IDListDemon> AREDL = {};
    inline static std::vector<IDDemonPack> AREDL_PACKS = {};
    inline static std::vector<IDListDemon> PEMONLIST = {};
    inline static bool AREDL_LOADED = false;
    inline static bool PEMONLIST_LOADED = false;

    static void isOk(std::string const&, geode::EventListener<geode::utils::web::WebTask>&&, bool, std::function<void(bool, int)> const&);
    static void loadAREDL(
        geode::EventListener<geode::utils::web::WebTask>&&,
        geode::EventListener<geode::utils::web::WebTask>&&,
        LoadingCircle*,
        std::function<void()> const&
    );
    static void loadAREDLPacks(
        geode::EventListener<geode::utils::web::WebTask>&&,
        geode::EventListener<geode::utils::web::WebTask>&&,
        LoadingCircle*,
        std::function<void()> const&
    );
    static void loadPemonlist(
        geode::EventListener<geode::utils::web::WebTask>&&,
        geode::EventListener<geode::utils::web::WebTask>&&,
        LoadingCircle*,
        std::function<void()> const&
    );
};
