#include <random>
#include "IDListLayer.hpp"

IDListLayer* IDListLayer::create() {
    auto ret = new IDListLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCScene* IDListLayer::scene() {
    auto ret = CCScene::create();
    ret->addChild(IDListLayer::create());
    return ret;
}

bool IDListLayer::init() {
    if (!CCLayer::init()) return false;

    setID("IDListLayer");
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto bg = CCSprite::create("GJ_gradientBG.png");
    auto& bgSize = bg->getTextureRect().size;
    bg->setAnchorPoint({ 0.0f, 0.0f });
    bg->setScaleX((winSize.width + 10.0f) / bgSize.width);
    bg->setScaleY((winSize.height + 10.0f) / bgSize.height);
    bg->setPosition({ -5.0f, -5.0f });
    bg->setColor({ 51, 51, 51 });
    addChild(bg);

    auto bottomLeftCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomLeftCorner->setPosition({ -1.0f, -1.0f });
    bottomLeftCorner->setAnchorPoint({ 0.0f, 0.0f });
    addChild(bottomLeftCorner);

    auto bottomRightCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomRightCorner->setPosition({ winSize.width + 1.0f, -1.0f });
    bottomRightCorner->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightCorner->setFlipX(true);
    addChild(bottomRightCorner);

    m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_countLabel->setAnchorPoint({ 1.0f, 1.0f });
    m_countLabel->setScale(0.6f);
    m_countLabel->setPosition(winSize.width - 7.0f, winSize.height - 3.0f);
    addChild(m_countLabel);

    m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 190.0f, 358.0f),
        PEMONLIST ? "Pemonlist" : "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getContentSize() / 2);
    addChild(m_list);

    addSearchBar();

    auto menu = CCMenu::create();
    menu->setPosition(0.0f, 0.0f);
    addChild(menu);

    m_backButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) { keyBackClicked(); });
    m_backButton->setPosition(25.0f, winSize.height - 25.0f);
    menu->addChild(m_backButton);

    m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto) { page(m_page - 1); });
    m_leftButton->setPosition(24.0f, winSize.height / 2);
    menu->addChild(m_leftButton);

    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto) { page(m_page + 1); });
    m_rightButton->setPosition(winSize.width - 24.0f, winSize.height / 2);
    menu->addChild(m_rightButton);

    m_infoButton = InfoAlertButton::create(PEMONLIST ? "Pemonlist" : "All Rated Extreme Demons List", PEMONLIST ? PEMONLIST_INFO : AREDL_INFO, 1.0f);
    m_infoButton->setPosition(30.0f, 30.0f);
    menu->addChild(m_infoButton, 2);

    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto& refreshBtnSize = refreshBtnSpr->getContentSize();
    auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto) {
        if (PEMONLIST) IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this] { populateList(m_query); });
        else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this] { populateList(m_query); });
    });
    refreshButton->setPosition(winSize.width - refreshBtnSize.width / 2 - 4.0f, refreshBtnSize.height / 2 + 4.0f);
    menu->addChild(refreshButton, 2);

    auto listToggler = CCMenuItemExt::createTogglerWithFrameName("GJ_moonsIcon_001.png", "GJ_starsIcon_001.png", 1.1f, [this](auto) {
        PEMONLIST = !PEMONLIST;
        if (PEMONLIST) {
            IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this] { page(0); });
            m_infoButton->m_title = "Pemonlist";
            m_infoButton->m_description = PEMONLIST_INFO;
        }
        else {
            IntegratedDemonlist::loadAREDL(std::move(m_listener), [this] { page(0); });
            m_infoButton->m_title = "All Rated Extreme Demons List";
            m_infoButton->m_description = AREDL_INFO;
        }
    });
    listToggler->toggle(PEMONLIST);
    listToggler->setPosition(30.0f, 60.0f);
    menu->addChild(listToggler, 2);

    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    m_pageLabel = CCLabelBMFont::create("1", "bigFont.fnt");
    m_pageLabel->setScale(0.8f);
    m_pageLabel->setPosition(pageBtnSpr->getContentSize() / 2);
    pageBtnSpr->addChild(m_pageLabel);
    m_pageButton = CCMenuItemExt::createSpriteExtra(pageBtnSpr, [this](auto) {
        auto popup = SetIDPopup::create(m_page + 1, 1, getMaxPage() + 1, "Go to Page", "Go", true, 1, 60.0f, false, false);
        popup->m_delegate = this;
        popup->show();
    });
    m_pageButton->setPositionY(winSize.height - 39.5f);
    menu->addChild(m_pageButton);
    // Sprite by Cvolton
    m_randomButton = CCMenuItemExt::createSpriteExtraWithFilename("BI_randomBtn_001.png"_spr, 0.9f, [this](auto) {
        static std::mt19937 mt(std::random_device{}());
        page(std::uniform_int_distribution<int>(0, getMaxPage())(mt));
    });
    m_randomButton->setPositionY(m_pageButton->getPositionY() - m_pageButton->getContentSize().height / 2 - m_randomButton->getContentSize().height / 2 - 5.0f);
    menu->addChild(m_randomButton);

    auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    lastArrow->setFlipX(true);
    auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherLastArrow->setPosition(lastArrow->getContentSize() / 2 + CCPoint { 20.0f, 0.0f });
    otherLastArrow->setFlipX(true);
    lastArrow->addChild(otherLastArrow);
    lastArrow->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) { page(getMaxPage()); });
    m_lastButton->setPositionY(m_randomButton->getPositionY() - m_randomButton->getContentSize().height / 2 - m_lastButton->getContentSize().height / 2 - 5.0f);
    menu->addChild(m_lastButton);
    auto x = winSize.width - m_randomButton->getContentSize().width / 2 - 3.0f;
    m_pageButton->setPositionX(x);
    m_randomButton->setPositionX(x);
    m_lastButton->setPositionX(x - 4.0f);

    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition(firstArrow->getContentSize() / 2 - CCPoint { 20.0f, 0.0f });
    firstArrow->addChild(otherFirstArrow);
    firstArrow->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) { page(0); });
    m_firstButton->setPosition(21.5f, m_lastButton->getPositionY());
    menu->addChild(m_firstButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->retain();
    m_loadingCircle->show();
    m_loadingCircle->setVisible(false);

    m_searchBarMenu->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);

    setKeyboardEnabled(true);
    if (PEMONLIST) {
        if (!IntegratedDemonlist::PEMONLIST.empty()) populateList("");
        else IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this] { populateList(""); });
    }
    else if (!IntegratedDemonlist::AREDL.empty()) populateList("");
    else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this] { populateList(""); });

    return true;
}

void IDListLayer::addSearchBar() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_searchBarMenu = CCMenu::create();
    m_searchBarMenu->setContentSize({ 358.0f, 30.0f });
    m_searchBarMenu->setPosition(0.0f, 190.0f);
    m_list->addChild(m_searchBarMenu);

    m_searchBarMenu->addChild(CCLayerColor::create({ 194, 114, 62, 255 }, 358.0f, 30.0f));

    if (!m_query.empty()) {
        auto searchButton = CCMenuItemExt::createSpriteExtraWithFilename("ID_findBtnOn_001.png"_spr, 0.7f, [this](auto) { search(); });
        searchButton->setPosition(337.0f, 15.0f);
        m_searchBarMenu->addChild(searchButton);
    } else {
        auto searchButton = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) { search(); });
        searchButton->setPosition(337.0f, 15.0f);
        m_searchBarMenu->addChild(searchButton);
    }

    m_searchBar = TextInput::create(413.3f, "Search Demons...");
    m_searchBar->setCommonFilter(CommonFilter::Any);
    m_searchBar->setPosition(165.0f, 15.0f);
    m_searchBar->setTextAlign(TextInputAlign::Left);
    m_searchBar->getInputNode()->setLabelPlaceholderScale(0.53f);
    m_searchBar->getInputNode()->setMaxLabelScale(0.53f);
    m_searchBar->setScale(0.75f);
    m_searchBar->setCallback([this](std::string const& text) { m_searchBarText = text; });
    m_searchBarMenu->addChild(m_searchBar);
}

void IDListLayer::populateList(std::string query) {
    m_pageLabel->setString(std::to_string(m_page + 1).c_str());
    m_loadingCircle->setVisible(true);
    m_list->m_listView->setVisible(false);
    m_searchBarMenu->setVisible(false);
    m_countLabel->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);
    m_fullSearchResults.clear();

    auto& list = PEMONLIST ? IntegratedDemonlist::PEMONLIST : IntegratedDemonlist::AREDL;
    if (!query.empty()) {
        auto queryLowercase = string::toLower(query);
        for (auto const& level : list) {
            if (string::startsWith(string::toLower(level.name), queryLowercase)) m_fullSearchResults.push_back(std::to_string(level.id));
        }
    } else {
        for (auto const& level : list) {
            m_fullSearchResults.push_back(std::to_string(level.id));
        }
    }

    m_query = query;

    if (m_fullSearchResults.empty()) {
        loadLevelsFinished(CCArray::create(), "");
        m_countLabel->setString("");
    }
    else {
        auto glm = GameLevelManager::sharedState();
        glm->m_levelManagerDelegate = this;
        auto searchResults = std::vector<std::string>(m_fullSearchResults.begin() + m_page * 10,
            m_fullSearchResults.begin() + std::min((int)m_fullSearchResults.size(), (m_page + 1) * 10));
        auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, string::join(searchResults, ","));
        auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());
        if (storedLevels) {
            loadLevelsFinished(storedLevels, "");
            setupPageInfo("", "");
        }
        else glm->getOnlineLevels(searchObject);
    }
}

int IDListLayer::getMaxPage() {
    auto size = m_fullSearchResults.size();
    return (size != 0 ? size % 10 == 0 ? size : size + (10 - (size % 10)) : 10) / 10 - 1;
}

void IDListLayer::loadLevelsFinished(CCArray* levels, const char*) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (m_list->getParent() == this) removeChild(m_list);
    m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 190.0f, 358.0f),
        PEMONLIST ? "Pemonlist" : "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getContentSize() / 2);
    addChild(m_list);
    addSearchBar();
    m_searchBar->setString(m_searchBarText);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    if (m_fullSearchResults.size() > 10) {
        auto maxPage = getMaxPage();
        m_leftButton->setVisible(m_page > 0);
        m_rightButton->setVisible(m_page < maxPage);
        m_firstButton->setVisible(m_page > 0);
        m_lastButton->setVisible(m_page < maxPage);
        m_pageButton->setVisible(true);
        m_randomButton->setVisible(true);
    }
}

void IDListLayer::loadLevelsFailed(const char*) {
    m_searchBarMenu->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create("Load Failed", "Failed to load levels. Please try again later.", "OK")->show();
}

void IDListLayer::setupPageInfo(gd::string, const char*) {
    m_countLabel->setString(fmt::format("{} to {} of {}", m_page * 10 + 1,
        std::min((int)m_fullSearchResults.size(), (m_page + 1) * 10), m_fullSearchResults.size()).c_str());
    m_countLabel->limitLabelWidth(100.0f, 0.6f, 0.0f);
}

void IDListLayer::search() {
    if (m_query != m_searchBarText) {
        if (PEMONLIST) IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this] {
            m_page = 0;
            populateList(m_searchBarText);
        });
        else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this] {
            m_page = 0;
            populateList(m_searchBarText);
        });
    }
}

void IDListLayer::page(int page) {
    auto maxPage = getMaxPage() + 1;
    m_page = (maxPage + (page % maxPage)) % maxPage;
    populateList(m_query);
}

void IDListLayer::keyDown(enumKeyCodes key) {
    switch (key)
    {
        case KEY_Left:
        case CONTROLLER_Left:
            if (m_leftButton->isVisible()) page(m_page - 1);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_rightButton->isVisible()) page(m_page + 1);
            break;
        case KEY_Escape:
        case CONTROLLER_B:
            keyBackClicked();
            break;
        default:
            CCLayer::keyDown(key);
            break;
    }
}

void IDListLayer::keyBackClicked() {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

void IDListLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::min(std::max(page - 1, 0), getMaxPage());
    populateList(m_query);
}

IDListLayer::~IDListLayer() {
    CC_SAFE_RELEASE(m_loadingCircle);
}
