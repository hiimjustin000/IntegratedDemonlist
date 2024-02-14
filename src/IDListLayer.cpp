#include "IDListLayer.hpp"
#include <Geode/utils/web.hpp>

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
    return size + (pad - (size % pad));
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

bool IDListLayer::init() {
    if (!CCLayer::init()) return false;

    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    auto bg = CCSprite::create("GJ_gradientBG.png");
    auto bgSize = bg->getTextureRect().size;

    bg->setAnchorPoint({ 0.0f, 0.0f });
    bg->setScaleX((winSize.width + 10.0f) / bgSize.width);
    bg->setScaleY((winSize.height + 10.0f) / bgSize.height);
    bg->setPosition({ -5.0f, -5.0f });
    bg->setColor({ 51, 51, 51 });
    this->addChild(bg);

    m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_countLabel->setAnchorPoint({ 1.0f, 1.0f });
    m_countLabel->setScale(0.6f);
    m_countLabel->setPosition(director->getScreenRight() - 7.0f, director->getScreenTop() - 3.0f);
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
    m_leftMenu->setPosition(director->getScreenLeft() + 24.0f, winSize.height / 2);
    auto leftBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(IDListLayer::onLeft));
    m_leftMenu->addChild(leftBtn);
    this->addChild(m_leftMenu);

    m_rightMenu = CCMenu::create();
    m_rightMenu->setPosition(director->getScreenRight() - 24.0f, winSize.height / 2);
    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    auto rightBtn = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(IDListLayer::onRight));
    m_rightMenu->addChild(rightBtn);
    this->addChild(m_rightMenu);

    m_infoMenu = CCMenu::create();
    m_infoMenu->setPosition(director->getScreenLeft() + 30.0f, director->getScreenBottom() + 30.0f);
    auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(IDListLayer::onInfo));
    m_infoMenu->addChild(infoBtn);
    this->addChild(m_infoMenu);

    m_loadingCircle = LoadingCircle::create();
    this->addChild(m_loadingCircle);

    m_searchBarView->setVisible(false);
    m_searchBar->setVisible(false);

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
    m_loadingCircle->show();
    m_list->m_listView->setVisible(false);
    m_searchBarView->setVisible(false);
    m_searchBar->setVisible(false);
    m_countLabel->setVisible(false);
    auto maxPage = paddedSize(m_fullSearchResults.size(), 10) / 10;
    m_leftMenu->setEnabled(m_page != 0);
    m_rightMenu->setEnabled(m_page != maxPage);
    m_leftMenu->setVisible(m_page != 0);
    m_rightMenu->setEnabled(m_page != maxPage);
    if (query.compare(m_query) != 0 && !query.empty()) {
        auto queryLowercase = toLowerCase(query);
        m_fullSearchResults = {};
        for (int i = 0; i < AREDL.size(); i++) {
            if (toLowerCase(AREDL_NAMES[i]).rfind(queryLowercase, 0) != std::string::npos) m_fullSearchResults.push_back(AREDL[i]);
        }
    }
    m_query = query;
    if (query.empty()) m_fullSearchResults = AREDL;
    auto searchResults = std::vector<int>(
        m_fullSearchResults.begin() + m_page * 10,
        m_fullSearchResults.begin() + std::min(static_cast<int>(m_fullSearchResults.size()), (m_page + 1) * 10)
    );
    m_countLabel->setString(fmt::format("{} to {} of {}", m_page * 10 + 1, (m_page + 1) * 10, m_fullSearchResults.size()).c_str());
    m_countLabel->updateLabel();
    m_countLabel->setScale(0.6f);
    if (m_countLabel->getScaledContentSize().width > 100.0f) m_countLabel->setScale(60.0f / m_countLabel->getScaledContentSize().width);
    auto glm = GameLevelManager::sharedState();
    glm->m_levelManagerDelegate = this;
    glm->getOnlineLevels(GJSearchObject::create(SearchType::MapPackOnClick, join(searchResults, ",")));
}

void IDListLayer::loadLevelsFinished(cocos2d::CCArray* levels, const char*) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    this->removeChild(m_list);
    m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 190.0f, 358.0f), "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 358.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2 - m_list->getScaledContentSize() / 2);
    this->addChild(m_list);
    addSearchBar();
    m_searchBar->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->fadeAndRemove();
    m_loadingCircle->setVisible(false);
}

void IDListLayer::onExit(CCObject*) {
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MapPack))));
}

void IDListLayer::onSearch(CCObject*) {
    m_page = 0;
    populateList(m_searchBar->getString());
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