#include "IDPackCell.hpp"

IDPackCell* IDPackCell::create(IDDemonPack pack) {
    auto ret = new IDPackCell();
    if (ret->init(pack)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IDPackCell::init(IDDemonPack pack) {
    if (!CCLayer::init()) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto difficultySprite = CCSprite::createWithSpriteFrameName("difficulty_10_btn_001.png");
    difficultySprite->setPosition({ 31.0f, 50.0f });
    difficultySprite->setScale(1.1f);
    addChild(difficultySprite, 2);

    auto nameLabel = CCLabelBMFont::create(pack.name.c_str(), "bigFont.fnt");
    nameLabel->setPosition(162.0f, 80.0f);
    nameLabel->limitLabelWidth(205.0f, 0.9f, 0.0f);
    addChild(nameLabel);

    auto viewSprite = ButtonSprite::create("View", 50, 0, 0.6f, false, "bigFont.fnt", "GJ_button_01.png", 50.0f);
    auto viewMenu = CCMenu::create(CCMenuItemExt::createSpriteExtra(viewSprite, [this, pack](auto) {
        std::vector<std::string> levels;
        for (auto level : pack.levels) levels.push_back(std::to_string(level));
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f,
            LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MapPackOnClick, string::join(levels, ",")))));
    }), nullptr);
    viewMenu->setPosition(347.0f - viewSprite->getContentWidth() / 2, 50.0f);
    addChild(viewMenu);

    auto progressBackground = CCSprite::create("GJ_progressBar_001.png");
    progressBackground->setColor({ 0, 0, 0 });
    progressBackground->setOpacity(125);
    progressBackground->setScaleX(0.6f);
    progressBackground->setScaleY(0.8f);
    progressBackground->setPosition({ 164.0f, 48.0f });
    addChild(progressBackground, 3);

    auto progressBar = CCSprite::create("GJ_progressBar_001.png");
    progressBar->setColor({ 184, 0, 0 });
    progressBar->setScaleX(0.985f);
    progressBar->setScaleY(0.83f);
    progressBar->setAnchorPoint({ 0.0f, 0.5f });
    auto rect = progressBar->getTextureRect();
    progressBar->setPosition({ rect.size.width * 0.0075f, progressBackground->getContentHeight() / 2 });
    auto gsm = GameStatsManager::sharedState();
    auto completed = std::count_if(pack.levels.begin(), pack.levels.end(), [gsm](auto& level) { return gsm->hasCompletedOnlineLevel(level); });
    progressBar->setTextureRect({ rect.origin.x, rect.origin.y, rect.size.width * (completed / (float)pack.levels.size()), rect.size.height });
    progressBackground->addChild(progressBar, 1);

    auto progressLabel = CCLabelBMFont::create(fmt::format("{}/{}", completed, pack.levels.size()).c_str(), "bigFont.fnt");
    progressLabel->setPosition({ 164.0f, 48.0f });
    progressLabel->setScale(0.5f);
    addChild(progressLabel, 4);

    auto pointsLabel = CCLabelBMFont::create(fmt::format("{} Points", pack.points).c_str(), "bigFont.fnt");
    pointsLabel->setPosition(164.0f, 20.0f);
    pointsLabel->setScale(0.7f);
    pointsLabel->setColor({ 255, 255, (unsigned char)(completed >= pack.levels.size() ? 50 : 255) });
    addChild(pointsLabel, 1);

    if (completed >= pack.levels.size()) {
        auto completedSprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        completedSprite->setPosition({ 250.0f, 49.0f });
        addChild(completedSprite, 5);
    }

    return true;
}
