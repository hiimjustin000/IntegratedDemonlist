#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

struct IDListDemon {
    int id;
    std::string name;
    int position;
};

class IntegratedDemonlist {
public:
    inline static std::vector<IDListDemon> AREDL = {};
    inline static std::vector<IDListDemon> PEMONLIST = {};
    inline static bool TRIED_LOADING = false;

    static void initializeDemons(web::WebResponse*, bool);
    static void loadAREDL();
    static void loadAREDL(EventListener<web::WebTask>&&, MiniFunction<void()> callback);
    static void loadPemonlist();
    static void loadPemonlist(EventListener<web::WebTask>&&, MiniFunction<void()> callback);
};
