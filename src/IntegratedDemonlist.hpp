#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

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
    inline static bool TRIED_LOADING = false;

    static void initializeDemons(web::WebResponse*, bool);
    static void isOk(std::string const&, EventListener<web::WebTask>&&, MiniFunction<void(bool, int)> callback);
    static void loadAREDL();
    static void loadAREDL(EventListener<web::WebTask>&&, EventListener<web::WebTask>&&, LoadingCircle*, MiniFunction<void()> callback);
    static void loadAREDLPacks(EventListener<web::WebTask>&&, EventListener<web::WebTask>&&, LoadingCircle*, MiniFunction<void()> callback);
    static void loadPemonlist();
    static void loadPemonlist(EventListener<web::WebTask>&&, EventListener<web::WebTask>&&, LoadingCircle*, MiniFunction<void()> callback);
};
