#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class IDListLayer : public CCLayer, LevelManagerDelegate {
public:
    inline static std::vector<int> AREDL = {};
    inline static std::vector<std::string> AREDL_NAMES = {};
    inline static std::vector<int> AREDL_POSITIONS = {};
    static IDListLayer* create();
    template<typename T>
    inline static std::vector<T> pluck(matjson::Value const& json, std::string const& key) {
        std::vector<T> ret;
        for (auto const& val : json.as_array()) {
            if (!val.contains("legacy") || !val["legacy"].as_bool()) ret.push_back(val[key].as<T>());
        }
        return ret;
    };

    void onExit(CCObject*);
    void onSearch(CCObject*);
    void onLeft(CCObject*);
    void onRight(CCObject*);
    void onInfo(CCObject*);
protected:
    GJListLayer* m_list;
    CCMenu* m_backMenu;
    CCMenu* m_leftMenu;
    CCMenu* m_rightMenu;
    CCMenu* m_infoMenu;
    CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    CCLayerColor* m_searchBarView;
    CCTextInputNode* m_searchBar;
    CCLabelBMFont* m_countLabel;
    int m_page = 0;
    std::string m_query = "";
    std::vector<int> m_fullSearchResults;

    bool init();
    void addSearchBar();
    void populateList(std::string query);
    void loadLevelsFinished(cocos2d::CCArray* levels, const char*);
    void loadLevelsFailed(const char*) {}
    void loadLevelsFinished(cocos2d::CCArray* levels, const char* test, int) {
        loadLevelsFinished(levels, test);
    }
    void loadLevelsFailed(const char* test, int) {}
    void setupPageInfo(std::string, const char*) {}
};