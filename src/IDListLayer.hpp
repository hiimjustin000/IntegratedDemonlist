#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class IDListLayer : public CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
public:
    inline static std::vector<int> AREDL = {};
    inline static std::vector<std::string> AREDL_NAMES = {};
    inline static std::vector<int> AREDL_POSITIONS = {};
    inline static bool AREDL_TRIED_LOADING = false;
    inline static std::unordered_map<std::string, web::WebTask> RUNNING_REQUESTS = {};

    static IDListLayer* create();
    static CCScene* scene();
    template<class T>
    static std::vector<T> pluck(matjson::Array const&, std::string const&);
    static void loadAREDL(bool, MiniFunction<void()> callback = []() {});
    static float createGap(CCNode*, CCNode*, float);

    void search();
    void page(int);
    void keyDown(enumKeyCodes) override;
    void keyBackClicked() override;

    ~IDListLayer();
protected:
    GJListLayer* m_list;
    CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    CCLayerColor* m_searchBarView;
    CCTextInputNode* m_searchBar;
    CCLabelBMFont* m_countLabel;
    CCLabelBMFont* m_pageLabel;
    CCMenuItemSpriteExtra* m_backButton;
    CCMenuItemSpriteExtra* m_leftButton;
    CCMenuItemSpriteExtra* m_rightButton;
    CCMenuItemSpriteExtra* m_infoButton;
    CCMenuItemSpriteExtra* m_refreshButton;
    CCMenuItemSpriteExtra* m_pageButton;
    CCMenuItemSpriteExtra* m_randomButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;
    int m_page = 0;
    std::string m_query = "";
    std::vector<std::string> m_fullSearchResults;

    bool init() override;
    void addSearchBar();
    void populateList(std::string query);
    int getMaxPage();
    void loadLevelsFinished(CCArray*, const char*) override;
    void loadLevelsFailed(const char*) override;
    void loadLevelsFinished(CCArray* levels, const char* key, int) override {
        loadLevelsFinished(levels, key);
    }
    void loadLevelsFailed(const char* key, int) override {
        loadLevelsFailed(key);
    }
    void setupPageInfo(gd::string, const char*) override;
    void setIDPopupClosed(SetIDPopup*, int) override;
};
