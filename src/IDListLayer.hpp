#include "IntegratedDemonlist.hpp"

class IDListLayer : public CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
private:
    inline static bool PEMONLIST = false;
    inline static const char* AREDL_INFO =
        "The <cg>All Rated Extreme Demons List</c> (<cg>AREDL</c>) is an <cp>unofficial ranking</c> of all rated <cj>classic mode</c> <cr>extreme demons</c> in Geometry Dash.\n"
        "It is managed by <cy>iiLogan</c>, <cy>SEDTHEPRODIGY</c>, <cy>Megu</c>, and <cy>Minebox260</c>.";
    inline static const char* PEMONLIST_INFO =
        "The <cg>Pemonlist</c> is an <cp>unofficial ranking</c> of all rated <cj>platformer mode</c> <cr>demons</c> in Geometry Dash.\n"
        "It is managed by <cy>camila314</c>, <cy>Extatica</c>, and <cy>mariokirby1703</c>.";
public:
    static IDListLayer* create();
    static CCScene* scene();

    void search();
    void page(int);
    void keyDown(enumKeyCodes) override;
    void keyBackClicked() override;

    ~IDListLayer();
protected:
    EventListener<web::WebTask> m_listener;
    GJListLayer* m_list;
    CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    CCMenu* m_searchBarMenu;
    TextInput* m_searchBar;
    CCLabelBMFont* m_countLabel;
    CCLabelBMFont* m_pageLabel;
    InfoAlertButton* m_infoButton;
    CCMenuItemSpriteExtra* m_backButton;
    CCMenuItemSpriteExtra* m_leftButton;
    CCMenuItemSpriteExtra* m_rightButton;
    CCMenuItemSpriteExtra* m_pageButton;
    CCMenuItemSpriteExtra* m_randomButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;
    int m_page = 0;
    std::string m_query = "";
    std::string m_searchBarText = "";
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
