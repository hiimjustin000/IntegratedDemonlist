#include "IDListLayer.hpp"
#include <Geode/utils/web.hpp>

template<typename T>
std::vector<T> pluck(matjson::Value const& json, std::string const& key) {
    std::vector<T> ret;
    for (auto const& val : json.as_array()) {
        if (!val.contains("legacy") || !val["legacy"].as_bool()) ret.push_back(val[key].as<T>());
    }
    return ret;
};

std::string join(std::vector<int> const& vec, std::string const& delim) {
    std::string ret;
    for (auto const& i : vec) {
        ret += std::to_string(i) + delim;
    }
    if (!ret.empty()) ret = ret.substr(0, ret.size() - delim.size());
    return ret;
}

std::vector<std::string> split(std::string const& str, std::string const& delim) {
    size_t pos_start = 0, pos_end, delim_len = delim.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = str.find(delim, pos_start)) != std::string::npos) {
        token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(str.substr(pos_start));
    return res;
}

std::string toLowerCase(std::string str) {
    auto strCopy = std::string(str);
    std::transform(strCopy.begin(), strCopy.end(), strCopy.begin(), ::tolower);
    return strCopy;
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

void IDListLayer::loadAREDL(utils::MiniFunction<void()> callback) {
    web::AsyncWebRequest()
        .get("https://api.aredl.net/api/aredl/list")
        .json()
        .then([callback](matjson::Value const& json) {
            AREDL = pluck<int>(json, "level_id");
            AREDL_NAMES = pluck<std::string>(json, "name");
            AREDL_POSITIONS = pluck<int>(json, "position");
            callback();
        })
        .expect([](std::string const& error) {
            FLAlertLayer::create("Load Failed", "Failed to load AREDL. Please try again later.", "OK")->show();
        });
}

bool IDListLayer::init() {
    if (!CCLayer::init()) return false;
    this->setID("IDListLayer"_spr);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto bg = CCSprite::create("GJ_gradientBG.png");
    auto bgSize = bg->getTextureRect().size;

    bg->setAnchorPoint({ 0.0f, 0.0f });
    bg->setScaleX((winSize.width + 10.0f) / bgSize.width);
    bg->setScaleY((winSize.height + 10.0f) / bgSize.height);
    bg->setPosition({ -5.0f, -5.0f });
    bg->setColor({ 51, 51, 51 });
    this->addChild(bg);

    auto bottomLeftCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomLeftCorner->setPosition({ -1.0f, -1.0f });
    bottomLeftCorner->setAnchorPoint({ 0.0f, 0.0f });
    this->addChild(bottomLeftCorner);

    auto bottomRightCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomRightCorner->setPosition({ winSize.width + 1.0f, -1.0f });
    bottomRightCorner->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightCorner->setFlipX(true);
    this->addChild(bottomRightCorner);

    m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_countLabel->setAnchorPoint({ 1.0f, 1.0f });
    m_countLabel->setScale(0.6f);
    m_countLabel->setPosition(winSize.width - 7.0f, winSize.height - 3.0f);
    this->addChild(m_countLabel);

    m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 190.0f, 358.0f), "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getScaledContentSize() / 2);
    this->addChild(m_list);

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
    this->addChild(m_searchBar);

    m_backMenu = CCMenu::create();
    auto backBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this, menu_selector(IDListLayer::onExit));
    backBtn->setPosition(-winSize.width / 2 + 25.0f, winSize.height / 2 - 25.0f);
    m_backMenu->addChild(backBtn);
    this->addChild(m_backMenu);

    m_leftMenu = CCMenu::create();
    m_leftMenu->setPosition(24.0f, winSize.height / 2);
    auto leftBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(IDListLayer::onLeft));
    m_leftMenu->addChild(leftBtn);
    this->addChild(m_leftMenu);

    m_rightMenu = CCMenu::create();
    m_rightMenu->setPosition(winSize.width - 24.0f, winSize.height / 2);
    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    auto rightBtn = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(IDListLayer::onRight));
    m_rightMenu->addChild(rightBtn);
    this->addChild(m_rightMenu);

    m_infoMenu = CCMenu::create();
    m_infoMenu->setPosition(30.0f, 30.0f);
    m_infoMenu->setZOrder(2);
    auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(IDListLayer::onInfo));
    m_infoMenu->addChild(infoBtn);
    this->addChild(m_infoMenu);

    m_refreshMenu = CCMenu::create();
    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto y = refreshBtnSpr->getContentSize().height / 2 + 4;
    m_refreshMenu->setPosition(CCDirector::sharedDirector()->getScreenRight() - y, y);
    m_refreshMenu->setZOrder(2);
    auto refreshBtn = CCMenuItemSpriteExtra::create(refreshBtnSpr, this, menu_selector(IDListLayer::onRefresh));
    refreshBtn->setID("demonlist-button"_spr);
    m_refreshMenu->addChild(refreshBtn);
    this->addChild(m_refreshMenu);

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
    this->addChild(m_rightSearchMenu);
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
    this->addChild(m_leftSearchMenu);

    m_loadingCircle = LoadingCircle::create();
    this->addChild(m_loadingCircle);

    m_searchBarView->setVisible(false);
    m_searchBar->setVisible(false);
    m_leftSearchMenu->setVisible(false);
    m_leftSearchMenu->setEnabled(false);
    m_rightSearchMenu->setVisible(false);
    m_rightSearchMenu->setEnabled(false);

    setKeyboardEnabled(true);
    populateList("");

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
    m_searchBarView->addChild(searchBarBg);
    m_searchBarView->addChild(searchBarMenu);

    m_list->addChild(m_searchBarView);
}

void IDListLayer::populateList(std::string query) {
    m_pageLabel->setString(std::to_string(m_page + 1).c_str());
    m_pageLabel->updateLabel();
    m_loadingCircle->setVisible(true);
    m_loadingCircle->show();
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
    if (query.compare(m_query) != 0 && !query.empty()) {
        auto queryLowercase = toLowerCase(query);
        m_fullSearchResults = {};
        for (int i = 0; i < AREDL.size(); i++) {
            if (toLowerCase(AREDL_NAMES[i]).rfind(queryLowercase, 0) != std::string::npos) m_fullSearchResults.push_back(AREDL[i]);
        }
    }
    m_query = query;
    if (query.empty()) m_fullSearchResults = AREDL;
    auto minimum = std::min(static_cast<int>(m_fullSearchResults.size()), (m_page + 1) * 10);
    auto searchResults = std::vector<int>(m_fullSearchResults.begin() + m_page * 10, m_fullSearchResults.begin() + minimum);
    m_countLabel->setString(fmt::format("{} to {} of {}", m_page * 10 + 1, minimum, m_fullSearchResults.size()).c_str());
    m_countLabel->updateLabel();
    m_countLabel->setScale(0.6f);
    if (m_countLabel->getScaledContentSize().width > 100.0f) m_countLabel->setScale(60.0f / m_countLabel->getScaledContentSize().width);
    auto glm = GameLevelManager::sharedState();
    glm->m_levelManagerDelegate = this;
    auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, join(searchResults, ","));
    auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());
    if (storedLevels) this->loadLevelsFinished(storedLevels, "");
    else glm->getOnlineLevels(searchObject);
}

void IDListLayer::loadLevelsFinished(CCArray* levels, const char*) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (m_list->getParent() == this) this->removeChild(m_list);
    m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 190.0f, 358.0f), "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getScaledContentSize() / 2);
    this->addChild(m_list);
    addSearchBar();
    m_searchBar->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->fadeAndRemove();
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
    m_countLabel->setVisible(true);
    m_loadingCircle->fadeAndRemove();
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create("Load Failed", "Failed to load levels. Please try again later.", "OK")->show();
}

void IDListLayer::onExit(CCObject*) {
    auto scene = CCScene::create();
    auto gm = GameManager::sharedState();
#ifdef GEODE_IS_WINDOWS
    auto onlineType = *(int*)((uintptr_t)gm + 0x3d4);
#elif defined(GEODE_IS_MACOS)
    auto onlineType = *(int*)((uintptr_t)gm + 0x4b8);
#elif defined(GEODE_IS_ANDROID32)
    auto onlineType = *(int*)((uintptr_t)gm + 0x3bc);
#elif defined(GEODE_IS_ANDROID64)
    auto onlineType = *(int*)((uintptr_t)gm + 0x4d8);
#else
    auto onlineType = 0;
#endif

    scene->addChild(LevelSearchLayer::create(onlineType));
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
}

void IDListLayer::onSearch(CCObject*) {
    if (m_query.compare(m_searchBar->getString()) != 0) {
        loadAREDL([this]() {
            m_page = 0;
            populateList(m_searchBar->getString());
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
    loadAREDL([this]() {
        populateList(m_query);
    });
}

void IDListLayer::onPage(CCObject*) {
    auto popup = SetIDPopup::create(m_page + 1, 1, paddedSize(m_fullSearchResults.size(), 10) / 10, "Go to Page", "Go", true, 1, 60.0f, false, false);
    popup->m_delegate = this;
    popup->show();
}

void IDListLayer::onRandom(CCObject*) {
    m_page = rand() % (paddedSize(m_fullSearchResults.size(), 10) / 10);
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
            if (m_leftMenu->isVisible()) this->onLeft(nullptr);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_rightMenu->isVisible()) this->onRight(nullptr);
            break;
        case KEY_Escape:
        case CONTROLLER_B:
            this->onExit(nullptr);
            break;
        default:
            CCLayer::keyDown(key);
            break;
    }
}

void IDListLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::min(std::max(page - 1, 0), paddedSize(m_fullSearchResults.size(), 10) / 10 - 1);
    populateList(m_query);
}