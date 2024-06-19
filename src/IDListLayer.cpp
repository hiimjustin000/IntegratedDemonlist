#include <random>
#include <Geode/utils/string.hpp>
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

    m_searchBar = CCTextInputNode::create(340.0f, 30.0f, "Search Demons...", "bigFont.fnt");
    m_searchBar->setAllowedChars(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
    m_searchBar->setLabelPlaceholderColor({ 150, 150, 150 });
    m_searchBar->setLabelPlaceholderScale(0.4f);
    m_searchBar->setMaxLabelScale(0.4f);
    m_searchBar->m_textField->setAnchorPoint({ 0.0f, 0.5f });
    m_searchBar->m_placeholderLabel->setAnchorPoint({ 0.0f, 0.5f });
    m_searchBar->setPosition(winSize.width / 2 - 160.f, winSize.height / 2 + 95.f);
    m_searchBar->setZOrder(60);
    addChild(m_searchBar);

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

    auto infoButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_infoIcon_001.png", 1.0f, [](auto) {
        FLAlertLayer::create(PEMONLIST ? "Pemonlist" : "AREDL", PEMONLIST ?
            "The <cg>Pemonlist</c> is an unofficial ranking of all rated <cj>platformer mode</c> <cr>Demons</c> in Geometry Dash.\n"
            "It is managed by <cy>camila314</c>, <cy>Extatica</c>, and <cy>mariokirby1703</c>." :
            "The <cg>All Rated Extreme Demons List</c> (AREDL) is an unofficial ranking of all rated <cj>classic mode</c> <cr>Extreme Demons</c> in Geometry Dash.\n"
            "It is managed by <cy>iiLogan</c>, <cy>SEDTHEPRODIGY</c>, <cy>Megu</c>, and <cy>Minebox260</c>.",
            "OK")->show();
    });
    infoButton->setPosition(30.0f, 30.0f);
    menu->addChild(infoButton, 2);

    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto& refreshBtnSize = refreshBtnSpr->getContentSize();
    auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto) {
        if (PEMONLIST) IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this]() { populateList(m_query); });
        else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this]() { populateList(m_query); });
    });
    refreshButton->setPosition(winSize.width - refreshBtnSize.width / 2 - 4.0f, refreshBtnSize.height / 2 + 4.0f);
    menu->addChild(refreshButton, 2);

    auto listToggler = CCMenuItemExt::createTogglerWithFrameName("GJ_moonsIcon_001.png", "GJ_starsIcon_001.png", 1.1f, [this](auto) {
        PEMONLIST = !PEMONLIST;
        if (PEMONLIST) IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this]() { page(0); });
        else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this]() { page(0); });
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
    auto randomBtnSpr = CCSprite::create("BI_randomBtn_001.png"_spr);
    randomBtnSpr->setScale(0.9f);
    m_randomButton = CCMenuItemExt::createSpriteExtra(randomBtnSpr, [this](auto) {
        std::random_device os_seed;
        const unsigned int seed = os_seed();
        std::mt19937 generator(seed);
        std::uniform_int_distribution<int> distribute(0, getMaxPage());
        page(distribute(generator));
    });
    m_randomButton->setPositionY(m_pageButton->getPositionY() - m_pageButton->getContentSize().height / 2 - m_randomButton->getContentSize().height / 2 - 5.0f);
    menu->addChild(m_randomButton);
    // oh boy
    // https://github.com/Cvolton/betterinfo-geode/blob/v4.0.0/src/hooks/LevelBrowserLayer.cpp#L118
    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    firstArrow->setPosition({ 35, 25 });
    firstArrow->setFlipX(true);
    auto secondArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    secondArrow->setPosition({ 15, 25 });
    secondArrow->setFlipX(true);
    auto arrowParent = CCNode::create();
    arrowParent->setContentSize({ 50, 50 });
    arrowParent->addChild(firstArrow);
    arrowParent->addChild(secondArrow);
    arrowParent->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(arrowParent, [this](auto) { page(getMaxPage()); });
    m_lastButton->setPositionY(m_randomButton->getPositionY() - m_randomButton->getContentSize().height / 2 - m_lastButton->getContentSize().height / 2 - 2.0f);
    menu->addChild(m_lastButton);
    auto x = winSize.width - 3.0f - m_randomButton->getContentSize().width / 2;
    m_pageButton->setPositionX(x);
    m_randomButton->setPositionX(x);
    m_lastButton->setPositionX(x);
    // https://github.com/Cvolton/betterinfo-geode/blob/v4.0.0/src/hooks/LevelBrowserLayer.cpp#L164
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition({ 35, 34.5 });
    auto otherSecondArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherSecondArrow->setPosition({ 15, 34.5 });
    auto otherArrowParent = CCNode::create();
    otherArrowParent->setContentSize({ 50, 69 });
    otherArrowParent->addChild(otherFirstArrow);
    otherArrowParent->addChild(otherSecondArrow);
    otherArrowParent->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(otherArrowParent, [this](auto) { page(0); });
    m_firstButton->setPosition(17.5f, winSize.height - 64.0f);
    menu->addChild(m_firstButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->retain();
    m_loadingCircle->show();
    m_loadingCircle->setVisible(false);

    m_searchBarView->setVisible(false);
    m_searchBar->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);

    setKeyboardEnabled(true);
    if (PEMONLIST) {
        if (!IntegratedDemonlist::PEMONLIST.empty()) populateList("");
        else IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this]() { populateList(""); });
    }
    else {
        if (!IntegratedDemonlist::AREDL.empty()) populateList("");
        else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this]() { populateList(""); });
    }

    return true;
}

void IDListLayer::addSearchBar() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_searchBarView = CCLayerColor::create({ 194, 114, 62, 255 }, 358.0f, 30.0f);
    m_searchBarView->setPosition(0.0f, 190.0f);
    auto searchBarMenu = CCMenu::create();
    searchBarMenu->setPosition(337.0f, 15.0f);
    auto searchButtonSpr = CCSprite::createWithSpriteFrameName("gj_findBtn_001.png");
    searchButtonSpr->setScale(0.7f);
    auto searchButton = CCMenuItemExt::createSpriteExtra(searchButtonSpr, [this](auto) { search(); });
    searchBarMenu->addChild(searchButton);
    auto searchBarBg = CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    searchBarBg->setPosition(165.0f, 15.0f);
    searchBarBg->setContentSize({ 620.0f, 40.0f });
    searchBarBg->setColor({ 126, 59, 7 });
    searchBarBg->setScale(0.5f);
    m_searchBarView->addChild(searchBarMenu);
    m_searchBarView->addChild(searchBarBg);

    m_list->addChild(m_searchBarView);
}

void IDListLayer::populateList(std::string query) {
    m_pageLabel->setString(std::to_string(m_page + 1).c_str());
    m_loadingCircle->setVisible(true);
    m_list->m_listView->setVisible(false);
    m_searchBarView->setVisible(false);
    m_searchBar->setVisible(false);
    m_countLabel->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);
    m_fullSearchResults.clear();

    auto& list = PEMONLIST ? IntegratedDemonlist::PEMONLIST : IntegratedDemonlist::AREDL;
    if (query != m_query && !query.empty()) {
        auto queryLowercase = string::toLower(query);
        for (auto const& level : list) {
            if (string::startsWith(string::toLower(level.name), queryLowercase)) m_fullSearchResults.push_back(std::to_string(level.id));
        }
    }
    m_query = query;
    if (query.empty()) {
        for (auto const& level : list) {
            m_fullSearchResults.push_back(std::to_string(level.id));
        }
    }

    if (m_fullSearchResults.empty()) {
        loadLevelsFinished(CCArray::create(), "");
        m_countLabel->setString("");
    }
    else {
        auto glm = GameLevelManager::sharedState();
        glm->m_levelManagerDelegate = this;
        auto searchResults = std::vector<std::string>(m_fullSearchResults.begin() + m_page * 10,
            m_fullSearchResults.begin() + std::min(static_cast<int>(m_fullSearchResults.size()), (m_page + 1) * 10));
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
    return (size % 10 == 0 ? size : size + (10 - (size % 10))) / 10 - 1;
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
    m_searchBar->setVisible(true);
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
    m_searchBar->setVisible(true);
    m_searchBarView->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create("Load Failed", "Failed to load levels. Please try again later.", "OK")->show();
}

void IDListLayer::setupPageInfo(gd::string, const char*) {
    m_countLabel->setString(fmt::format("{} to {} of {}", m_page * 10 + 1,
        std::min(static_cast<int>(m_fullSearchResults.size()), (m_page + 1) * 10), m_fullSearchResults.size()).c_str());
    m_countLabel->limitLabelWidth(100.0f, 0.6f, 0.0f);
}

void IDListLayer::search() {
    auto searchString = m_searchBar->getString();
    if (m_query != searchString) {
        if (PEMONLIST) IntegratedDemonlist::loadPemonlist(std::move(m_listener), [this, searchString]() {
            m_page = 0;
            populateList(searchString);
        });
        else IntegratedDemonlist::loadAREDL(std::move(m_listener), [this, searchString]() {
            m_page = 0;
            populateList(searchString);
        });
    }
}

void IDListLayer::page(int page) {
    auto maxPage = getMaxPage() + 1;
    m_page = (maxPage + (page % maxPage)) % maxPage;
    populateList(m_query);
}

void IDListLayer::deselectKeyboard() {
    m_searchBar->onClickTrackNode(false);
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
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, LevelSearchLayer::scene(GameManager::sharedState()->m_unkSize4_17)));
}

void IDListLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::min(std::max(page - 1, 0), getMaxPage());
    populateList(m_query);
}

IDListLayer::~IDListLayer() {
    CC_SAFE_RELEASE(m_loadingCircle);
}
