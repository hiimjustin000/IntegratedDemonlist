#include "IntegratedDemonlist.hpp"

class IDListLayer : public CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
private:
    inline static bool PEMONLIST = false;
public:
    static IDListLayer* create();
    static CCScene* scene();

    void search();
    void page(int);
    void deselectKeyboard();
    void keyDown(enumKeyCodes) override;
    void keyBackClicked() override;

    ~IDListLayer();
protected:
    EventListener<web::WebTask> m_listener;
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
