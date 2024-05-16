#include <random>
#include <Geode/utils/string.hpp>
#include <Geode/utils/web.hpp>
#include "IDListLayer.hpp"

template<typename T>
std::vector<T> pluck(matjson::Value const& json, std::string const& key) {
    std::vector<T> ret;
    for (auto const& val : json.as_array()) {
        if ((!val.contains("legacy") || !val["legacy"].as_bool()) && !val["two_player"].as_bool()) ret.push_back(val[key].as<T>());
    }
    return ret;
}

int paddedSize(int size, int pad) {
    return size % pad == 0 ? size : size + (pad - (size % pad));
}

IDListLayer* IDListLayer::create() {
    auto ret = new IDListLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void IDListLayer::loadAREDL(bool fromMenuLayer, utils::MiniFunction<void()> callback) {
    web::AsyncWebRequest()
        .get("https://api.aredl.net/api/aredl/levels")
        .json()
        .then([fromMenuLayer, callback](matjson::Value const& json) {
            AREDL = pluck<int>(json, "level_id");
            AREDL_NAMES = pluck<std::string>(json, "name");
            AREDL_POSITIONS = pluck<int>(json, "position");
            if (fromMenuLayer) Notification::create("AREDL Loaded", NotificationIcon::Success)->show();
            callback();
        })
        .expect([fromMenuLayer](std::string const& error) {
            if (fromMenuLayer) Notification::create("AREDL Load Failed", NotificationIcon::Error)->show();
            else FLAlertLayer::create("Load Failed", "Failed to load AREDL. Please try again later.", "OK")->show();
        });
}

bool IDListLayer::init() {
    if (!CCLayer::init()) return false;
    setID("IDListLayer"_spr);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto bg = CCSprite::create("GJ_gradientBG.png");
    CCSize bgSize = bg->getTextureRect().size;

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

    m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 190.0f, 358.0f), "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getScaledContentSize() / 2);
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

    m_backMenu = CCMenu::create();
    auto backBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this, menu_selector(IDListLayer::onClose));
    backBtn->setPosition(-winSize.width / 2 + 25.0f, winSize.height / 2 - 25.0f);
    m_backMenu->addChild(backBtn);
    addChild(m_backMenu);

    m_leftMenu = CCMenu::create();
    m_leftMenu->setPosition(24.0f, winSize.height / 2);
    auto leftBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(IDListLayer::onLeft));
    m_leftMenu->addChild(leftBtn);
    addChild(m_leftMenu);

    m_rightMenu = CCMenu::create();
    m_rightMenu->setPosition(winSize.width - 24.0f, winSize.height / 2);
    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    auto rightBtn = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(IDListLayer::onRight));
    m_rightMenu->addChild(rightBtn);
    addChild(m_rightMenu);

    m_infoMenu = CCMenu::create();
    m_infoMenu->setPosition(30.0f, 30.0f);
    m_infoMenu->setZOrder(2);
    auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(IDListLayer::onInfo));
    m_infoMenu->addChild(infoBtn);
    addChild(m_infoMenu);

    m_refreshMenu = CCMenu::create();
    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto y = refreshBtnSpr->getContentSize().height / 2 + 4;
    m_refreshMenu->setPosition(CCDirector::sharedDirector()->getScreenRight() - y, y);
    m_refreshMenu->setZOrder(2);
    auto refreshBtn = CCMenuItemSpriteExtra::create(refreshBtnSpr, this, menu_selector(IDListLayer::onRefresh));
    m_refreshMenu->addChild(refreshBtn);
    addChild(m_refreshMenu);

    m_rightSearchMenu = CCMenu::create();
    m_rightSearchMenu->setPositionY(-39.5f);
    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    m_pageLabel = CCLabelBMFont::create("1", "bigFont.fnt");
    m_pageLabel->setScale(0.8f);
    m_pageLabel->setPosition(pageBtnSpr->getContentSize() / 2);
    pageBtnSpr->addChild(m_pageLabel);
    auto pageBtn = CCMenuItemSpriteExtra::create(pageBtnSpr, this, menu_selector(IDListLayer::onPage));
    pageBtn->setPositionY(winSize.height);
    m_rightSearchMenu->addChild(pageBtn);
    // Sprite by Cvolton
    auto randomBtnSpr = CCSprite::create("BI_randomBtn_001.png"_spr);
    randomBtnSpr->setScale(0.9f);
    auto randomBtn = CCMenuItemSpriteExtra::create(randomBtnSpr, this, menu_selector(IDListLayer::onRandom));
    randomBtn->setPositionY(winSize.height - randomBtn->getContentSize().height - 4.15f);
    m_rightSearchMenu->addChild(randomBtn);
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
    m_lastButton = CCMenuItemSpriteExtra::create(arrowParent, this, menu_selector(IDListLayer::onLast));
    m_lastButton->setPositionY(winSize.height - randomBtn->getContentSize().height - m_lastButton->getContentSize().height - 11.0f);
    m_rightSearchMenu->addChild(m_lastButton);
    m_rightSearchMenu->setPositionX(winSize.width - 3.0f - randomBtn->getContentSize().width / 2);
    addChild(m_rightSearchMenu);
    m_leftSearchMenu = CCMenu::create();
    m_leftSearchMenu->setPosition(17.5f, winSize.height - 64.0f);
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
    m_firstButton = CCMenuItemSpriteExtra::create(otherArrowParent, this, menu_selector(IDListLayer::onFirst));
    m_leftSearchMenu->addChild(m_firstButton);
    addChild(m_leftSearchMenu);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->retain();
    m_loadingCircle->show();
    m_loadingCircle->setVisible(false);

    m_searchBarView->setVisible(false);
    m_searchBar->setVisible(false);
    m_leftSearchMenu->setVisible(false);
    m_leftSearchMenu->setEnabled(false);
    m_rightSearchMenu->setVisible(false);
    m_rightSearchMenu->setEnabled(false);

    setKeyboardEnabled(true);
    if (!AREDL.empty()) populateList("");

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
    auto searchButton = CCMenuItemSpriteExtra::create(searchButtonSpr, this, menu_selector(IDListLayer::onSearch));
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
    m_leftMenu->setVisible(false);
    m_leftMenu->setEnabled(false);
    m_rightMenu->setVisible(false);
    m_rightMenu->setEnabled(false);
    m_firstButton->setVisible(false);
    m_firstButton->setEnabled(false);
    m_lastButton->setVisible(false);
    m_lastButton->setEnabled(false);
    m_leftSearchMenu->setVisible(false);
    m_leftSearchMenu->setEnabled(false);
    m_rightSearchMenu->setVisible(false);
    m_rightSearchMenu->setEnabled(false);
    m_fullSearchResults.clear();
    if (query.compare(m_query) != 0 && !query.empty()) {
        auto queryLowercase = string::toLower(query);
        for (int i = 0; i < AREDL.size(); i++) {
            if (string::toLower(AREDL_NAMES[i]).rfind(queryLowercase, 0) != std::string::npos) m_fullSearchResults.push_back(std::to_string(AREDL[i]));
        }
    }
    m_query = query;
    if (query.empty()) {
        for (int i = 0; i < AREDL.size(); i++) {
            m_fullSearchResults.push_back(std::to_string(AREDL[i]));
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

void IDListLayer::loadLevelsFinished(CCArray* levels, const char*) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (m_list->getParent() == this) removeChild(m_list);
    m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 190.0f, 358.0f), "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getContentSize() / 2);
    addChild(m_list);
    addSearchBar();
    m_searchBar->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    if (m_fullSearchResults.size() > 10) {
        auto maxPage = paddedSize(m_fullSearchResults.size(), 10) / 10 - 1;
        m_leftSearchMenu->setVisible(true);
        m_leftSearchMenu->setEnabled(true);
        m_rightSearchMenu->setVisible(true);
        m_rightSearchMenu->setEnabled(true);
        m_leftMenu->setEnabled(m_page > 0);
        m_rightMenu->setEnabled(m_page < maxPage);
        m_firstButton->setEnabled(m_page > 0);
        m_lastButton->setEnabled(m_page < maxPage);
        m_leftMenu->setVisible(m_page > 0);
        m_rightMenu->setVisible(m_page < maxPage);
        m_firstButton->setVisible(m_page > 0);
        m_lastButton->setVisible(m_page < maxPage);
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

void IDListLayer::onClose(CCObject*) {
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, LevelSearchLayer::scene(GameManager::sharedState()->m_unkSize4_17)));
}

void IDListLayer::onSearch(CCObject*) {
    auto searchString = m_searchBar->getString();
    if (m_query.compare(searchString) != 0) {
        loadAREDL(false, [this, searchString]() {
            m_page = 0;
            populateList(searchString);
        });
    }
}

void IDListLayer::onLeft(CCObject*) {
    m_page = (m_page - 1) % (paddedSize(m_fullSearchResults.size(), 10) / 10);
    populateList(m_query);
}

void IDListLayer::onRight(CCObject*) {
    m_page = (m_page + 1) % (paddedSize(m_fullSearchResults.size(), 10) / 10);
    populateList(m_query);
}

void IDListLayer::onInfo(CCObject*) {
    std::string line1 = "The <cg>All Rated Extreme Demons List</c> (AREDL) is an unofficial ranking of all rated <cr>Extreme Demons</c> in Geometry Dash.\n";
    std::string line2 = "It is managed by <cy>iiLogan</c>, <cy>SEDTHEPRODIGY</c>, <cy>Megu</c>, and <cy>Minebox260</c>.";
    FLAlertLayer::create("AREDL", line1 + line2, "OK")->show();
}

void IDListLayer::onRefresh(CCObject*) {
    loadAREDL(false, [this]() {
        populateList(m_query);
    });
}

void IDListLayer::onPage(CCObject*) {
    auto popup = SetIDPopup::create(m_page + 1, 1, paddedSize(m_fullSearchResults.size(), 10) / 10, "Go to Page", "Go", true, 1, 60.0f, false, false);
    popup->m_delegate = this;
    popup->show();
}

void IDListLayer::onRandom(CCObject*) {
    std::random_device os_seed;
    const unsigned int seed = os_seed();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribute(0, paddedSize(m_fullSearchResults.size(), 10) / 10 - 1);
    m_page = distribute(generator);
    populateList(m_query);
}

void IDListLayer::onFirst(CCObject*) {
    m_page = 0;
    populateList(m_query);
}

void IDListLayer::onLast(CCObject*) {
    m_page = paddedSize(m_fullSearchResults.size(), 10) / 10 - 1;
    populateList(m_query);
}

void IDListLayer::keyDown(enumKeyCodes key) {
    switch (key)
    {
        case KEY_Left:
        case CONTROLLER_Left:
            if (m_leftMenu->isVisible()) onLeft(nullptr);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_rightMenu->isVisible()) onRight(nullptr);
            break;
        case KEY_Escape:
        case CONTROLLER_B:
            onClose(nullptr);
            break;
        default:
            CCLayer::keyDown(key);
            break;
    }
}

void IDListLayer::keyBackClicked() {
    onClose(nullptr);
}

void IDListLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::min(std::max(page - 1, 0), paddedSize(m_fullSearchResults.size(), 10) / 10 - 1);
    populateList(m_query);
}

IDListLayer::~IDListLayer() {
    m_loadingCircle->release();
}
