//
// Created by FU-QAQ on 2024/12/18.
//

#include "UserPage.h"
#include "editinfopage.h"
// #include "../core/common-data/CommonData.h"
// #include "../effect-component/material-effect/MaterialEffect.h"

#include "ElaInteractiveCard.h"
#include "ElaToolButton.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "ElaApplication.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFontDatabase>
#include <QPainter>
#include <mutex>

// QMap<UserType,UserPage*> UserPage::_userObjMap;
static std::mutex m;

// UserPage *UserPage::getInstance(UserType type,UserInfo uInfo, GroupInfo gInfo, QWidget * parent) {
    // if(_userObjMap.isEmpty()){
        // m.lock();
        // if(_userObjMap.isEmpty()){
        //     UserInfo i;
        //     _userObjMap[UserType::Myself] = new UserPage(UserType::Myself,parent);
        //     _userObjMap[UserType::Strangers] = new UserPage(UserType::Strangers,parent);
        //     _userObjMap[UserType::Friends] = new UserPage(UserType::Friends,parent);
        //     _userObjMap[UserType::Group_Member] = new UserPage(UserType::Group_Member,parent);
        //     _userObjMap[UserType::Group_OP] = new UserPage(UserType::Group_OP,parent);
        //     _userObjMap[UserType::Group_Creater] = new UserPage(UserType::Group_Creater,parent);
        // }
        // m.unlock();
    // }
    // UserPage *p = _userObjMap[type];
    // if ((type == UserType::Myself   ||
    //      type == UserType::Strangers ||
    //      type == UserType::Friends)  &&
    //     !uInfo._ssid.isEmpty())
    //     p->setInfo(uInfo);
    // if ((type == UserType::Group_Member   ||
    //      type == UserType::Group_OP        ||
    //      type == UserType::Group_Creater)  &&
    //     !gInfo._ssid.isEmpty())
    //     p->setInfo(gInfo);
    // return p;
// }

// void UserPage::destroyUserPage() {
//     if(_userObjMap.isEmpty())
//         return ;
//     else{
        // m.lock();
        // delete _userObjMap[UserType::Myself];
        // delete _userObjMap[UserType::Strangers];
        // delete _userObjMap[UserType::Friends];
        // delete _userObjMap[UserType::Group_Member];
        // delete _userObjMap[UserType::Group_OP];
        // delete _userObjMap[UserType::Group_Creater];
//         _userObjMap.clear();
//         m.unlock();
//     }
// }

// UserPage::UserPage(UserType type,QWidget *parent) : ElaWidget(parent) {
//     _curType =  type;
//     _isGroup = (type==UserType::Group_Member  ||
//                 type==UserType::Group_OP      ||
//                 type==UserType::Group_Creater);

    // initWindow();

    // initEdgeLayout();

    // initContent();

    // initConnectFunc();

//     switch(type){
//     case Myself:
//         _editUserButton->show();
//         _likeButton->show();
//         _localInfoText->show();
//         _joinDayText->show();
//         _signContentText->show();

//         _addFriendButton->hide();
//         _sendMsgButton->hide();
//         _callButton->hide();
//         _groupNotice->hide();
//         _groupResume->hide();
//         _groupManagerButton->hide();
//         _editGroupButton->hide();
//         break;
//     case Strangers:
//         _addFriendButton->show();
//         _likeButton->show();
//         _localInfoText->show();
//         _joinDayText->show();
//         _signContentText->show();
//         _sendMsgButton->show();

//         _editUserButton->hide();
//         _callButton->hide();
//         _groupNotice->hide();
//         _groupResume->hide();
//         _groupManagerButton->hide();
//         _editGroupButton->hide();
//         break;
//     case Friends:
//         _callButton->show();
//         _likeButton->show();
//         _localInfoText->show();
//         _joinDayText->show();
//         _signContentText->show();
//         _sendMsgButton->show();

//         _addFriendButton->hide();
//         _editUserButton->hide();
//         _groupNotice->hide();
//         _groupResume->hide();
//         _groupManagerButton->hide();
//         _editGroupButton->hide();
//         break;
//     case Group_Member:
//     case Group_OP:
//     case Group_Creater:
//         _groupNotice->show();
//         _groupResume->show();
//         _sendMsgButton->show();

//         // hide friends component
//         _addFriendButton->hide();
//         _callButton->hide();
//         _editUserButton->hide();
//         _likeButton->hide();
//         _localInfoText->hide();
//         _joinDayText->hide();
//         _signContentText->hide();
//         _groupManagerButton->hide();
//         _editGroupButton->hide();

//         if (Group_OP || Group_Creater) {
//             _groupManagerButton->show();
//             _editGroupButton->show();
//         }
//         break;
//     }
// }

UserPage::~UserPage() {
}

void UserPage::initWindow() {
    eApp->init();
    QFontDatabase::addApplicationFont("/user-page/rc-page/ElaAwesome.ttf");
    setWindowButtonFlag(ElaAppBarType::StayTopButtonHint,false);
    setWindowButtonFlag(ElaAppBarType::ThemeChangeButtonHint,false);
    setWindowButtonFlag(ElaAppBarType::MinimizeButtonHint,false);
    setWindowButtonFlag(ElaAppBarType::MaximizeButtonHint,false);
    setWindowButtonFlag(ElaAppBarType::CloseButtonHint,false);
    setWindowFlag(Qt::Popup);

    // setGraphicsEffect(g_pCommonEffect->setShadowForWidgetBorder(10));
    setWindowTitle("");
    this->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    this->setIsFixedSize(true);
    this->setFixedSize(300,400);
    _avatarInfo         = new ElaInteractiveCard(this);
    _likeButton         = new ElaToolButton(this);
    _addFriendButton    = new ElaPushButton(this);
    _sendMsgButton      = new QPushButton(this);
    _callButton         = new ElaPushButton(this);
    _editUserButton     = new ElaPushButton(this);
    _remarkText         = new ElaText(this);
    _signContentText    = new ElaText(this);
    _joinDayText        = new ElaText(this);
    _localInfoText      = new ElaText(this);
    _groupResume        = new ElaText(this);
    _groupNotice        = new ElaPushButton(this);
    _editPage           = new EditInfoPage(this);
    _groupManagerButton = new ElaToolButton(this);
    _editGroupButton    = new ElaPushButton(this);

    _mainLayout         = new QVBoxLayout;
    _buttonLayout       = new QHBoxLayout;
    _textLayout         = new QGridLayout;
}

void UserPage::initEdgeLayout() {
    _textLayout->addWidget(_avatarInfo,0,0,3,2);
    _textLayout->addWidget(_likeButton,1,2,1,1);
    if (!_isGroup) {
        auto *day = new ElaText("加入天数",this);
        auto *remark = new ElaText("备注",this);
        auto *sign = new ElaText("个性签名",this);
        auto *local = new ElaText("所在地",this);
        day->setTextPixelSize(12);
        remark->setTextPixelSize(12);
        sign->setTextPixelSize(12);
        local->setTextPixelSize(12);
        sign->setTextStyle(ElaTextType::BodyStrong);
        remark->setTextStyle(ElaTextType::BodyStrong);
        day->setTextStyle(ElaTextType::BodyStrong);
        local->setTextStyle(ElaTextType::BodyStrong);
        _remarkText->setTextPixelSize(12);
        _signContentText->setTextPixelSize(12);
        _joinDayText->setTextPixelSize(12);
        _localInfoText->setTextPixelSize(12);
        _remarkText->setTextStyle(ElaTextType::Body);
        _signContentText->setTextStyle(ElaTextType::Body);
        _joinDayText->setTextStyle(ElaTextType::Body);
        _localInfoText->setTextStyle(ElaTextType::Body);
        _textLayout->addWidget(day,3,0,1,1);
        _textLayout->addWidget(remark,4,0,1,1);
        _textLayout->addWidget(sign,5,0,1,1);
        _textLayout->addWidget(local,6,0,1,1);
        _textLayout->addWidget(_joinDayText,3,1,1,1);
        _textLayout->addWidget(_remarkText,4,1,1,1);
        _textLayout->addWidget(_signContentText,5,1,1,1);
        _textLayout->addWidget(_localInfoText,6,1,1,1);
        _textLayout->setContentsMargins(10,0,0,0);
    }
    else{
        setFixedSize(300,350);

        auto *resume = new ElaText("介绍",this);
        auto *notice = new ElaText("群公告",this);
        auto *groupManager = new ElaText("群管理",this);
        resume->setTextPixelSize(12);
        notice->setTextPixelSize(12);
        groupManager->setTextPixelSize(12);
        resume->setTextStyle(ElaTextType::BodyStrong);
        notice->setTextStyle(ElaTextType::BodyStrong);
        groupManager->setTextStyle(ElaTextType::BodyStrong);
        QFont font;
        font.setPixelSize(12);
        _remarkText->setTextPixelSize(12);
        _groupResume->setTextPixelSize(12);
        _groupNotice->setFont(font);
        _remarkText->setTextStyle(ElaTextType::Body);
        _groupResume->setTextStyle(ElaTextType::Body);
        _textLayout->addWidget(resume,3,0,1,1);
        _textLayout->addWidget(notice,4,0,1,1);
        _textLayout->addWidget(groupManager,5,0,1,1);
        _textLayout->addWidget(_remarkText,2,1,1,1);
        _textLayout->addWidget(_groupResume,3,1,1,1);
        _textLayout->addWidget(_groupNotice,4,1,1,1);
        _textLayout->addWidget(_groupManagerButton,5,1,1,1);
        _textLayout->setContentsMargins(10,0,0,0);
    }

    // add | call | user edit | group edit | send
    _buttonLayout->setSpacing(20);
    _buttonLayout->insertWidget(0,_addFriendButton);
    _buttonLayout->insertWidget(1,_callButton);
    _buttonLayout->insertWidget(2,_editUserButton);
    _buttonLayout->insertWidget(3,_editGroupButton);
    _buttonLayout->insertWidget(4,_sendMsgButton);
    _buttonLayout->setContentsMargins(0,0,0,0);

    _mainLayout->addItem(_textLayout);
    _mainLayout->addItem(_buttonLayout);
    _mainLayout->setContentsMargins(20,0,20,20);

    this->setLayout(_mainLayout);
    this->setContentsMargins(0,0,0,0);
}

void UserPage::initContent() {
    _avatarInfo->setCardPixmap(QPixmap(":/user-page/rc-page/img/SS-default-icon.jpg"));
    _avatarInfo->setTitle("{test-name}");
    _avatarInfo->setSubTitle("SSID 1129935824");

    _likeButton->setElaIcon(ElaIconType::ThumbsUp);
    _likeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _likeButton->setText("9999+");
    _addFriendButton->setBorderRadius(10);
    _addFriendButton->setText("加好友");
    _addFriendButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _addFriendButton->setFixedHeight(40);

    _sendMsgButton->setText("发消息");
    _sendMsgButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _sendMsgButton->setFixedHeight(35);
    _sendMsgButton->setStyleSheet(R"(
        QPushButton {
            background-color : rgb(0,153,255);
            color : white;
            border-radius : 10px;
        }
        QPushButton:hover {
            background-color : rgb(0,141,235);
        }
        QPushButton:pressed {
            background-color : rgb(0,141,235);
        }
    )");

    _callButton->setBorderRadius(10);
    _callButton->setText("音视频通话");
    _callButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _callButton->setFixedHeight(40);

    _editUserButton->setBorderRadius(10);
    _editUserButton->setText("编辑资料");
    _editUserButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _editUserButton->setFixedHeight(40);

    _editGroupButton->setBorderRadius(10);
    _editGroupButton->setText("编辑群资料");
    _editGroupButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _editGroupButton->setFixedHeight(40);

    _groupManagerButton->setElaIcon(ElaIconType::Ballot);

    _remarkText->setText("{Remark}");
    _signContentText->setText("{Sign Text}");
    _localInfoText->setText("{Country/Province}");
    _joinDayText->setText("316");

    _editPage->hide();
}

void UserPage::initConnectFunc() {
    connect(_editUserButton,&ElaPushButton::clicked,[=]() {
        // UserBaseInfoDTO udto = g_pCommonData->getCurUserInfo();
        // UserInfo ifo{
        //     Myself,
        //     static_cast<int>((udto.createTime / (24 * 60 * 60 * 1000)) + 1),
        //     static_cast<int>(udto.thumbUpCount),
        //     udto.ssid,
        //     udto.username,
        //     "",
        //     udto.personalSign,
        //     udto.avatarPath,
        //     {
        //     }
        // };
        // _editPage->sltSetEditPageInfo(ifo);
        // _editPage->show();
        // _userObjMap[_curType]->hide();
        // emit _userObjMap[_curType]->sigShowArchPageMaskEffect();
    });

    // connect(_editGroupButton,&ElaPushButton::clicked,[=]() {
        // GroupBaseInfoDTO gdto = g_pCommonData->getGroupBaseInfoBySSID(_avatarInfo->getSubTitle());
        // QList<GroupMemberInfoDTO> mdto = g_pCommonData->getGroupMemberInfoData(_avatarInfo->getSubTitle(),-1,-1);
        // GroupInfo gfo{
        //     _curType,
        //     gdto.ssidGroup,
        //     gdto.groupName,
        //     "",
        //     gdto.profile,
        //     gdto.avatarPath,
        //     static_cast<int>(mdto.count()),
        //     {{"test notice"}}
        // };
        /*editPage->sltSetEditPageInfo(gfo,_curType);
        _editPage->show();
        _userObjMap[_curType]->hide();
        emit _userObjMap[_curType]->sigShowArchPageMaskEffect();*/
    // });

    // connect(_editPage,&EditInfoPage::sigEditPageClosed,this,[=]() {
    //     _editPage->hide();
    //     emit this->sigHideArchPageMaskEffect();
    // });
    // connect(_sendMsgButton,&QPushButton::clicked, this,[=]() {
    //     emit _userObjMap[_curType]->sigCommunicateRequestBySSID(_avatarInfo->getSubTitle());
    //     hide();
    // });
    // connect(_editPage,&EditInfoPage::sigUserAvatarChanged,this,&UserPage::sigUserAvatarChanged);
    // connect(_editPage,&EditInfoPage::sigUserInfoChanged,this,&UserPage::sigUserInfoChanged);
    // connect(_editPage,&EditInfoPage::sigGroupAvatarChanged,this,&UserPage::sigGroupAvatarChanged);
    // connect(_editPage,&EditInfoPage::sigGroupInfoChanged,this,&UserPage::sigGroupInfoChanged);
}

// void UserPage::setInfo(const UserInfo &info) {
//     if(!info.isEmpty()){
//         _avatarInfo->setTitle(info._name);
//         _avatarInfo->setSubTitle(info._ssid);
//         _avatarInfo->setCardPixmap(QPixmap(info._picPath.isEmpty()?":/user-page/rc-page/img/SS-default-icon.jpg":info._picPath));

//         _likeButton->setText(info._likeCount > 9999 ? QString("9999+"):QString::number(info._likeCount));
//         _joinDayText->setText(QString::number(info._joinDay));
//         _remarkText->setText(info._remark);
//         _signContentText->setText(info._signContent);

//         QString setLocalInfo;
//         if(!info._localInfo.province.isEmpty() && !info._localInfo.city.isEmpty()){
//             setLocalInfo = info._localInfo.city + "·" + info._localInfo.district;
//         }
//         else{
//             setLocalInfo = info._localInfo.province;
//         }
//         _localInfoText->setText(setLocalInfo);
//     }
// }

// void UserPage::setInfo(const GroupInfo &info) {
//     if(!info.isEmpty()){
//         _avatarInfo->setTitle(info._name);
//         _avatarInfo->setSubTitle(info._ssid + "  (" + QString::number(info._memberCount) +"人)");
//         _avatarInfo->setCardPixmap(QPixmap(info._picPath.isEmpty()?":/user-page/rc-page/img/SS-default-icon.jpg":info._picPath));

//         _remarkText->setText(info._remark);
//         _groupResume->setText(info._resume);
//         _groupNotice->setText(info._notices.first().content);
//     }
// }

void UserPage::showAt(const QPoint &pos) {
    move(pos);
    show();
}

void UserPage::moveUserEditPageToCenter(const QPoint &pos) {
    _editPage->move(pos);
}

// void UserPage::paintEvent(QPaintEvent *event) {
//     QPainter painter(this);
//     painter.setRenderHint(QPainter::Antialiasing);
//     QPainterPath path;
//     path.addRoundedRect(rect(), 10, 10);
//     painter.setClipPath(path);
//     painter.fillPath(path, Qt::white);
// }
// void UserPage::showEvent(QShowEvent *event) {
//     QWidget::showEvent(event);

//     QPainterPath path;
//     path.addRoundedRect(rect(), 10, 10);
//     setMask(QRegion(path.toFillPolygon().toPolygon()));
// }
