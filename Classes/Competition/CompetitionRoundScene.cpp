﻿#include "CompetitionRoundScene.h"
#include "Competition.h"
#include "../common.h"
#include "../widget/AlertView.h"

USING_NS_CC;

CompetitionRoundScene *CompetitionRoundScene::create(const std::shared_ptr<CompetitionData> &competitionData) {
    CompetitionRoundScene *ret = new (std::nothrow) CompetitionRoundScene();
    if (ret != nullptr && ret->initWithData(competitionData)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CompetitionRoundScene::initWithData(const std::shared_ptr<CompetitionData> &competitionData) {
    if (UNLIKELY(!BaseScene::initWithTitle("第N轮"))) {
        return false;
    }

    _competitionData = competitionData;

    _players.reserve(competitionData->players.size());
    std::transform(competitionData->players.begin(), competitionData->players.end(), std::back_inserter(_players),
        [](CompetitionPlayer &p) { return &p; });

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 排列座位按钮
    ui::Button *button = ui::Button::create("source_material/btn_square_highlighted.png", "source_material/btn_square_selected.png");
    this->addChild(button);
    button->setScale9Enabled(true);
    button->setContentSize(Size(55.0f, 20.0f));
    button->setTitleFontSize(12);
    button->setTitleText("排列座位");
    button->setPosition(Vec2(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height - 70.0f));
    button->addClickEventListener(std::bind(&CompetitionRoundScene::onRankButton, this, std::placeholders::_1));

    _colWidth[0] = visibleSize.width * 0.1f;
    _colWidth[1] = visibleSize.width * 0.1f;
    _colWidth[2] = visibleSize.width * 0.25f;
    _colWidth[3] = visibleSize.width * 0.1f;
    _colWidth[4] = visibleSize.width * 0.1f;
    _colWidth[5] = visibleSize.width * 0.15f;
    _colWidth[6] = visibleSize.width * 0.2f;

    _posX[0] = _colWidth[0] * 0.5f;
    _posX[1] = _posX[0] + _colWidth[0] * 0.5f + _colWidth[1] * 0.5f;
    _posX[2] = _posX[1] + _colWidth[1] * 0.5f + _colWidth[2] * 0.5f;
    _posX[3] = _posX[2] + _colWidth[2] * 0.5f + _colWidth[3] * 0.5f;
    _posX[4] = _posX[3] + _colWidth[3] * 0.5f + _colWidth[4] * 0.5f;
    _posX[5] = _posX[4] + _colWidth[4] * 0.5f + _colWidth[5] * 0.5f;
    _posX[6] = _posX[5] + _colWidth[5] * 0.5f + _colWidth[6] * 0.5f;

    const char *titleTexts[] = { "名次", "编号", "选手姓名", "桌号", "座次", "标准分", "比赛分" };
    for (int i = 0; i < 7; ++i) {
        Label *label = Label::createWithSystemFont(titleTexts[i], "Arail", 12);
        label->setColor(Color3B::BLACK);
        this->addChild(label);
        label->setPosition(Vec2(origin.x + _posX[i], visibleSize.height - 100.0f));
    }

    cw::TableView *tableView = cw::TableView::create();
    tableView->setContentSize(Size(visibleSize.width, visibleSize.height - 115.0f));
    tableView->setDelegate(this);
    tableView->setDirection(ui::ScrollView::Direction::VERTICAL);
    tableView->setVerticalFillOrder(cw::TableView::VerticalFillOrder::TOP_DOWN);

    tableView->setScrollBarPositionFromCorner(Vec2(5, 2));
    tableView->setScrollBarWidth(4);
    tableView->setScrollBarOpacity(0x99);
    tableView->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    tableView->setPosition(Vec2(origin.x + visibleSize.width * 0.5f, origin.y + visibleSize.height * 0.5f - 55.0f));
    tableView->reloadData();
    this->addChild(tableView);

    return true;
}

ssize_t CompetitionRoundScene::numberOfCellsInTableView(cw::TableView *table) {
    return _players.size();
}

cocos2d::Size CompetitionRoundScene::tableCellSizeForIndex(cw::TableView *table, ssize_t idx) {
    return Size(0, 30);
}

cw::TableViewCell *CompetitionRoundScene::tableCellAtIndex(cw::TableView *table, ssize_t idx) {
    typedef cw::TableViewCellEx<Label *[7], ui::EditBox *, LayerColor *[2]> CustomCell;
    CustomCell *cell = (CustomCell *)table->dequeueCell();

    if (cell == nullptr) {
        cell = CustomCell::create();

        CustomCell::ExtDataType &ext = cell->getExtData();
        Label *(&labels)[7] = std::get<0>(ext);
        ui::EditBox *&editBox = std::get<1>(ext);
        LayerColor *(&layerColor)[2] = std::get<2>(ext);

        Size visibleSize = Director::getInstance()->getVisibleSize();

        layerColor[0] = LayerColor::create(Color4B(0xC0, 0xC0, 0xC0, 0x10), visibleSize.width, 29);
        cell->addChild(layerColor[0]);

        layerColor[1] = LayerColor::create(Color4B(0x80, 0x80, 0x80, 0x10), visibleSize.width, 29);
        cell->addChild(layerColor[1]);

        for (int i = 0; i < 7; ++i) {
            Label *label = Label::createWithSystemFont("", "Arail", 12);
            label->setColor(Color3B::BLACK);
            cell->addChild(label);
            label->setPosition(Vec2(_posX[i], 15.0f));
            labels[i] = label;
        }
    }

    const CustomCell::ExtDataType ext = cell->getExtData();
    Label *const (&labels)[7] = std::get<0>(ext);
    ui::EditBox *editBox = std::get<1>(ext);
    LayerColor *const (&layerColor)[2] = std::get<2>(ext);

    layerColor[0]->setVisible(!(idx & 1));
    layerColor[1]->setVisible(!!(idx & 1));

    static const char *seatText[] = { "东", "南", "西", "北" };
    const CompetitionPlayer &player = *_players[idx];
    labels[0]->setString(std::to_string(idx + 1));
    labels[1]->setString(std::to_string(player.serial));
    labels[2]->setString(player.name);
    labels[3]->setString(std::to_string((idx >> 2) + 1));
    labels[4]->setString(seatText[idx & 3]);
    labels[5]->setString("0");
    labels[6]->setString("0");

    return cell;
}

void CompetitionRoundScene::onRankButton(cocos2d::Ref *sender) {
    Node *rootNode = Node::create();
    rootNode->setContentSize(Size(80, 120));

    ui::RadioButtonGroup *radioGroup = ui::RadioButtonGroup::create();
    rootNode->addChild(radioGroup);

    static const char *titles[] = { "随机", "编号蛇形", "排名蛇形", "高高碰", "自定义" };

    for (int i = 0; i < 5; ++i) {
        ui::RadioButton *radioButton = ui::RadioButton::create("source_material/btn_square_normal.png", "source_material/btn_square_highlighted.png");
        rootNode->addChild(radioButton);
        radioButton->setZoomScale(0.0f);
        radioButton->ignoreContentAdaptWithSize(false);
        radioButton->setContentSize(Size(20.0f, 20.0f));
        radioButton->setPosition(Vec2(10.0f, 110.0f - i * 25.0f));
        radioGroup->addRadioButton(radioButton);

        Label *label = Label::createWithSystemFont(titles[i], "Arial", 12);
        label->setColor(Color3B::BLACK);
        rootNode->addChild(label);
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(Vec2(25.0f, 110.0f - i * 25.0f));
    }

    AlertView::showWithNode("排列座位", rootNode, []() {
    }, nullptr);
}
