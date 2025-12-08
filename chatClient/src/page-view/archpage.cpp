#include "ArchPage.h"
#include "addpage.h"
#include "creategrouppage.h"
#include"utils/log/logfile.h"
#include"core/commondata.h"

// #include "../CommonFunc.hpp"
#include "aboutpage.h"
#include "messagepage.h"
#include "contactpage.h"
#include"landpage.h"
// #include "../settings-page/SettingsPage.h"
// #include "../file-manager-page/FileManagerPage.h"
// #include "../user-page/UserPage.h"
// #include "../llmapp-page/LLMAppPage.h"
#include "llm-page/LLMChatPage.h"
#include "ElaContentDialog.h"
#include "ElaStatusBar.h"
#include "ElaText.h"
#include "ElaToolBar.h"
#include "ElaToolButton.h"
#include "ElaSuggestBox.h"
#include "ElaMenu.h"

#include <QFile>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <mutex>

#include "effect-component/ssmaskwidget.h"
#include "effect-component/loadingdialog.h"
#include "core/database/dbcachehelper.h"

static std::mutex m;
ArchPage * ArchPage::_obj = nullptr;

ArchPage *ArchPage::getInstance() {
    if(!_obj){
        m.lock();
        if(!_obj){
            _obj = new ArchPage();
        }
        m.unlock();
    }
    return _obj;
}

void ArchPage::destroyInstance() {
    if (_obj) {
        m.lock();
        if (_obj) {
            _obj->deleteLater();
        }
        m.unlock();
    }
}


 void ArchPage::convertUserInfoToFriendshipData(const UserBaseInfoData& userBaseInfo, const QString& friList, const QString& groList)
{
    // 解析好友列表和群组列表
    QStringList friendList = friList.split('|', Qt::SkipEmptyParts);
    QStringList groupList = groList.split('|', Qt::SkipEmptyParts);
    
    // 清空缓存并重置计数器
    cache.clear();
    pendingCallbacks = 0;
    
    //处理好友数据
    for(const QString& friendId : friendList)
    {
        int friendssid=friendId.toInt();
        if(friendssid>0)
        {
            FriendshipData friendData;
            friendData.ssid=userBaseInfo.ssid;
            friendData.friendSSID=friendssid;
            friendData.friendType=1;
            friendData.groupingName="我的好友";//默认分组

            // 使用值捕获和索引来安全地更新数据
            int index = cache.size();
            cache.push_back(friendData);
            pendingCallbacks++;

            // 优化：使用DbCacheHelper缓存
            DbCacheHelper::instance().getUserInfo(friendssid, [this, index, friendssid](const UserBaseInfoData& userInfo) {
                if (index >= 0 && index < cache.size()) {
                    // 使用缓存数据
                    cache[index].friendName = userInfo.username;
                    cache[index].status = "在线"; // 默认状态
                }

                pendingCallbacks--;
                if (pendingCallbacks == 0) {
                    // 所有回调都完成了，发出信号
                    emit cacheUpdated();
                }
            });
        }
        else
        {
            SSLog::log(SSLog::LogLevel::SS_WARNING, QString(__FILE__), __LINE__, "Invalid friend ID format: " + friendId);
        }
    }
    //处理群组数据
    for(const QString& groupId:groupList)
    {
        int groupssid=groupId.toInt();
        if(groupssid>0)
        {
            FriendshipData groupData;
            groupData.ssid=groupId.toInt();
            groupData.friendSSID=groupssid;
            groupData.friendType=2;
            groupData.groupingName="我加入的群聊";

            // 使用值捕获和索引来安全地更新数据
            int index = cache.size();
            cache.push_back(groupData);
            pendingCallbacks++;

            // 优化：使用DbCacheHelper缓存
            DbCacheHelper::instance().getGroupInfo(groupssid, [this, index, groupssid](const GroupBaseInfoData& groupInfo, const QList<GroupMemberInfoData>& members) {
                if (index >= 0 && index < cache.size()) {
                    // 使用缓存数据
                    cache[index].friendName = groupInfo.groupName;
                    cache[index].status = "群聊";
                }
                pendingCallbacks--;
                if (pendingCallbacks == 0) {
                    // 所有回调都完成了，发出信号
                    emit cacheUpdated();
                }
            });
        }
        else
        {
            qDebug()<<"Invaild group ID format:"<<groupId;
        }
    }
}

//更新用户卡片
void ArchPage::sltTriggerUpdate(UserInfo* info) {
    // UserBaseInfoDTO udto = g_pCommonData->getCurUserInfo();
    // if (udto.username.isEmpty() || udto.username == "-1") {
        // setUserInfoCardTitle("{NULL}");
    // } else {
        // setUserInfoCardTitle(udto.username);
    // }
    // if (udto.avatarPath.isEmpty() || udto.avatarPath == "-1") {
    //     setUserInfoCardPixmap(QPixmap(":/arch-page/rc-page/img/SS-default-icon.jpg"));
    // } else {
    //     setUserInfoCardPixmap(udto.avatarPath);
    // }
    // if (udto.ssid.isEmpty() || udto.ssid == "-1") {
    //     setUserInfoCardSubTitle("{NULL}");
    // } else {
    //     setUserInfoCardSubTitle(udto.ssid);
    // }
    UserBaseInfoData userInfo = CommonData::getInstance()->getCurUserInfo();
    setUserInfoCardTitle(userInfo.username);
    setUserInfoCardSubTitle(QString::number(userInfo.ssid));
    setUserInfoCardPixmap(QPixmap(":/include/Image/Cirno.jpg"));
}

void ArchPage::sltShowMaskEffect() {
    _maskWidget->setVisible(true);
    _maskWidget->raise();
    _maskWidget->setFixedSize(this->size());
    _maskWidget->startMaskAnimation(90);
}

void ArchPage::sltHideMaskEffect() {
    _maskWidget->startMaskAnimation(0);
}

void ArchPage::sltShowLoading() {
    sltShowMaskEffect();
    _loadingDialog->setVisible(true);
}

void ArchPage::sltHideLoading() {
    _loadingDialog->setVisible(false);
    sltHideMaskEffect();
}

ArchPage::ArchPage(QWidget *parent) : ElaWindow(parent) {
    initWindow();

    initEdgeLayout();

    initContent();

    initConnectFunc();

    // intercept close event
    _closeDialog = new ElaContentDialog(this);
    connect(_closeDialog, &ElaContentDialog::rightButtonClicked, this, &ArchPage::closeWindow);
    connect(_closeDialog, &ElaContentDialog::middleButtonClicked, this, [=]() {
        _closeDialog->close();
        showMinimized();
    });
    this->setIsDefaultClosed(false);
    connect(this, &ArchPage::closeButtonClicked, this, [=]() {
        _closeDialog->exec();
    });

    moveToCenter();
}

ArchPage::~ArchPage() {
    delete _addPage;
    _addPage = nullptr;
    delete _createGroupPage;
    _createGroupPage = nullptr;
}

void ArchPage::initWindow() {
    setWindowIcon(QIcon(":/arch-page/rc-page/img/SS-default-icon.jpg"));
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setMinimumSize(400,500);
    resize(800, 650);

    // setWindowTitle("Synergy-Spot \t\t version:  " + QString(SS_VERSION));

    _statusBar       =   new ElaStatusBar(this);
    _statusText      =   new ElaText("初始化成功！", this);
    _toolBar         =   new ElaToolBar("Tool Bar", this);
    _addButton       =   new ElaToolButton(this);
    _searchSuggest   =   new ElaSuggestBox(this);

    _maskWidget      =   new SSMaskWidget(this);
    _loadingDialog   =   new LoadingDialog(this);
    _addPage         =   new AddPage();

    _maskWidget->setParent(this);
    _maskWidget->setVisible(false);
    _loadingDialog->setParent(this);
    _loadingDialog->setVisible(false);

    // init arch user pic and user name
}

void ArchPage::initEdgeLayout() {
    // status bar
    _statusText->setTextPixelSize(14);
    _statusBar->addWidget(_statusText);

    // toolbar
    _toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    _toolBar->setToolBarSpacing(5);
    _toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _toolBar->setIconSize(QSize(25,25));

    // suggest and add
    _searchSuggest->setMinimumWidth(200);
    _searchSuggest->setMaximumWidth(this->maximumWidth());
    _searchSuggest->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _searchSuggest->setContentsMargins(0,0,0,0);

    _addButton->setFixedSize(60,40);
    _addButton->setBorderRadius(8);

    _toolBar->addWidget(_searchSuggest);
    _toolBar->addWidget(_addButton);

    addToolBar(Qt::TopToolBarArea, _toolBar);
}

void ArchPage::initContent() {
    // message page 第三个参数是消息条数
    addPageNode("Message", g_pMessagePage, _msgNoticeNum, ElaIconType::Comment);

    // contact page
    addPageNode("Contact", g_pContactPage, _contactNoticeNum, ElaIconType::User);

    // open webui page
    // addPageNode("LLMApp", g_pLLMAppPage, _llmAppNum, ElaIconType::Code);

    // file manager
    // addFooterNode("FileManager", g_pFileManagerPage, _fileManagerKey, 0, ElaIconType::Folders);

    // setting
    // addFooterNode("Settings", g_pSettingsPage, _settingsKey, 0, ElaIconType::GearComplex);

    // about
    // addFooterNode("About", nullptr, _aboutKey, 0, ElaIconType::CircleInfo);

    addPageNode("LLMApp",g_pLLMChatPage,_llmAppNum,ElaIconType::Code);

    _addButton->setElaIcon(ElaIconType::CirclePlus);
    _addButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    _addButton->setIsTransparent(false);
    auto * addMenu      =   new ElaMenu(_addButton);
    _createAction       =   addMenu->addElaIconAction(ElaIconType::CommentPlus, "创建群聊");
    _addAction          =   addMenu->addElaIconAction(ElaIconType::UserPlus, "添加好友/群聊");
    _addButton->setMenu(addMenu);
    _searchSuggest->setPlaceholderText("Search...");

    setStatusBar(_statusBar);

    _maskWidget->setVisible(false);
    _maskWidget->move(0,0);
    _maskWidget->setFixedSize(this->size());

    // g_pSettingsPage->setMainWindowWidget(this);
}

void ArchPage::initConnectFunc() {
    connect(this, &ElaWindow::navigationNodeClicked, this, [=](ElaNavigationType::NavigationNodeType nodeType, QString nodeKey) {
        if (_aboutKey == nodeKey)
        {
            g_pAboutPage->setFixedSize(400, 300);
            g_pAboutPage->moveToCenter();
            g_pAboutPage->show();
        }
    });
    // connect(this, &ElaWindow::userInfoCardClicked, this, [=](){
    //     UserBaseInfoDTO udto = g_pCommonData->getCurUserInfo();
    //     UserInfo ifo{
    //         Myself,
    //         static_cast<int>((udto.createTime / (24 * 60 * 60 * 1000)) + 1),
    //         static_cast<int>(udto.thumbUpCount),
    //         udto.ssid,
    //         udto.username,
    //         "",
    //         udto.personalSign,
    //         udto.avatarPath,
    //         {
    //         }
    //     };
    //     UserPage * wid = g_pUserPage(UserType::Myself,ifo,{});

    //     QPoint globalPos = QCursor::pos();
    //     wid->showAt(globalPos + QPoint{10,10});
    // });

    // user edit page require mask effect
    // connect(g_pUserPage(UserType::Myself,{},{}),&UserPage::sigShowArchPageMaskEffect,this,[=]() {
    //     sltShowMaskEffect();
    //     g_pUserPage(UserType::Myself,{},{})->moveUserEditPageToCenter(pos() + QPoint(width()/2,height()/2) - QPoint(200,300));
    // });
    // connect(g_pUserPage(UserType::Group_OP,{},{}),&UserPage::sigShowArchPageMaskEffect,this,[=]() {
    //     sltShowMaskEffect();
    //     g_pUserPage(UserType::Group_OP,{},{})->moveUserEditPageToCenter(pos() + QPoint(width()/2,height()/2) - QPoint(200,300));
    // });
    // connect(g_pUserPage(UserType::Group_Creater,{},{}),&UserPage::sigShowArchPageMaskEffect,this,[=]() {
    //     sltShowMaskEffect();
    //     g_pUserPage(UserType::Group_Creater,{},{})->moveUserEditPageToCenter(pos() + QPoint(width()/2,height()/2) - QPoint(200,300));
    // });
    // connect(g_pUserPage(UserType::Myself,{},{}),&UserPage::sigHideArchPageMaskEffect,this,&ArchPage::sltHideMaskEffect);
    // connect(g_pUserPage(UserType::Group_OP,{},{}),&UserPage::sigHideArchPageMaskEffect,this,&ArchPage::sltHideMaskEffect);
    // connect(g_pUserPage(UserType::Group_Creater,{},{}),&UserPage::sigHideArchPageMaskEffect,this,&ArchPage::sltHideMaskEffect);

    // // refresh data
    // connect(g_pCommonData,&CommonData::sigUpdateAvatarData,this,&ArchPage::sltTriggerUpdate);

    // add friend
    connect(_addAction,&QAction::triggered,this,[=]() {
        sltShowMaskEffect();
        _addPage->show();
    });

    // // add page request mask effect
    connect(_addPage,&AddPage::sigHideArchPageMaskEffect,this,&ArchPage::sltHideMaskEffect);

    // // create group
    connect(_createAction,&QAction::triggered,this,[=]() {
        if (_createGroupPage == nullptr) {
            _createGroupPage = new CreateGroupPage();
            // create group page request mask effect
            connect(_createGroupPage,&CreateGroupPage::sigHideArchPageMaskEffect,this,&ArchPage::sltHideMaskEffect);
        }

        sltShowMaskEffect();
        _createGroupPage->show();
        emit g_pContactPage->sigTriggerAddToCreateGroupList({});
    });

    connect(this,&ArchPage::sigJumpOtherPageRequest,this,[=](PageName pageName) {
        switch (pageName) {
        case PageName::MessagePage:
            //根据pagekey跳转到不同的界面
            navigation(g_pMessagePage->property("ElaPageKey").toString());
            break;
        case PageName::ContactPage:
            navigation(g_pContactPage->property("ElaPageKey").toString());
            break;
    //     case LLMPage:
    //         navigation(g_pLLMAppPage->property("ElaPageKey").toString());
    //         break;
    //     case FileManagerPage:
    //         navigation(g_pFileManagerPage->property("ElaPageKey").toString());
    //         break;
    //     case SettingsPage:
    //         navigation(g_pSettingsPage->property("ElaPageKey").toString());
    //         break;
        }
    });

    connect(g_pContactPage,&ContactPage::sigHideArchPageMaskEffect,this,&ArchPage::sltHideMaskEffect);
    connect(g_pContactPage,&ContactPage::sigShowArchPageMaskEffect,this,&ArchPage::sltShowMaskEffect);
    
    // 连接缓存更新信号
    connect(this, &ArchPage::cacheUpdated, this, [this]() {
        SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "Cache updated, size: " + QString::number(cache.size()));
        // for(auto f:cache)
        // {
        //    SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__, "好友信息: " + f.friendName + " " + QString::number(f.friendSSID) + " " + QString::number(f.friendType) + " " + QString::number(f.ssid) + " " + f.status);
        // }
        ContactPage::getInstance()->loadCacheContact(cache);
    });
}

void ArchPage::resizeEvent(QResizeEvent *event) {
    _maskWidget->setFixedSize(event->size());
    ElaWindow::resizeEvent(event);
}
