#include <Geode/Geode.hpp>

using namespace geode::prelude;

class IDListLayer : public CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
public:
    inline static std::vector<int> AREDL = {};
    inline static std::vector<std::string> AREDL_NAMES = {};
    inline static std::vector<int> AREDL_POSITIONS = {};
    static IDListLayer* create();
    static void loadAREDL() {
        loadAREDL([]() {});
    }
    static void loadAREDL(MiniFunction<void()> callback);

    void onExit(CCObject*);
    void onSearch(CCObject*);
    void onLeft(CCObject*);
    void onRight(CCObject*);
    void onInfo(CCObject*);
    void onRefresh(CCObject*);
    void onPage(CCObject*);
    void onRandom(CCObject*);
    void onFirst(CCObject*);
    void onLast(CCObject*);
    void keyDown(enumKeyCodes) override;
protected:
    GJListLayer* m_list;
    CCMenu* m_backMenu;
    CCMenu* m_leftMenu;
    CCMenu* m_rightMenu;
    CCMenu* m_infoMenu;
    CCMenu* m_refreshMenu;
    CCMenu* m_leftSearchMenu;
    CCMenu* m_rightSearchMenu;
    CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    CCLayerColor* m_searchBarView;
    CCTextInputNode* m_searchBar;
    CCLabelBMFont* m_countLabel;
    CCLabelBMFont* m_pageLabel;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;
    int m_page = 0;
    std::string m_query = "";
    std::vector<int> m_fullSearchResults;

    bool init() override;
    void addSearchBar();
    void populateList(std::string query);
    void loadLevelsFinished(CCArray* levels, const char*) override;
    void loadLevelsFailed(const char*) override;
    void loadLevelsFinished(CCArray* levels, const char* test, int) override {
        loadLevelsFinished(levels, test);
    }
    void loadLevelsFailed(const char* test, int) override {
        loadLevelsFailed(test);
    }
    void setupPageInfo(gd::string, const char*) override {}
    void setIDPopupClosed(SetIDPopup*, int) override;
};