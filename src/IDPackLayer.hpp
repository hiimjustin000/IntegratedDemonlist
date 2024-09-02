#include "IDPackCell.hpp"

class IDPackLayer : public CCLayer, SetIDPopupDelegate {
private:
    inline static const char* AREDL_PACK_INFO =
        "The <cg>All Rated Extreme Demons List</c> (<cg>AREDL</c>) has <cp>packs</c> of <cr>extreme demons</c> that are <cj>related</c> in some way.\n"
        "If all levels in a pack are <cl>completed</c>, the pack can earn <cy>points</c> on <cg>aredl.net</c>.";
public:
    static IDPackLayer* create();
    static CCScene* scene();

    void search();
    void page(int);
    void keyDown(enumKeyCodes) override;
    void keyBackClicked() override;

    ~IDPackLayer() override;
protected:
    EventListener<web::WebTask> m_aredlListener;
    EventListener<web::WebTask> m_aredlOkListener;
    GJListLayer* m_list;
    CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    CCMenu* m_searchBarMenu;
    TextInput* m_searchBar;
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
    std::string m_searchBarText = "";
    std::vector<IDDemonPack> m_fullSearchResults;

    bool init() override;
    void addSearchBar();
    void showLoading();
    void populateList(std::string query);
    void setIDPopupClosed(SetIDPopup*, int) override;
};
