#include "contactpage.h"
#include "contactdelegate.h"
#include "contactmodel.h"

#include "ElaTreeView.h"
#include "ElaToolButton.h"
#include "ElaPivot.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QStandardItem>
#include <mutex>
#include <ElaTheme.h>

#include <core/clientrequesthandler.h>

ContactPage * ContactPage::_contactPage = nullptr;
static std::mutex m;

ContactPage *ContactPage::getInstance() {
    if(_contactPage == nullptr){
        m.lock();
        if(_contactPage == nullptr){
            _contactPage = new ContactPage();
        }
        m.unlock();
    }
    return _contactPage;
}

void ContactPage::destroyContactPage() {
    if(_contactPage == nullptr)
        return ;
    else{
        m.lock();
        delete _contactPage;
        _contactPage = nullptr;
        m.unlock();
    }
}

//添加好友分组
void ContactPage::addFriendGrouping(const QString &name) {
    if (_friendModel != nullptr && !_groupingInfos["friend"].contains(name)) {
        _friendModel->addGrouping(name);
    }
    _groupingInfos["friend"].insert(name,{});
}

//获取好友分组
// QList<QString> ContactPage::getFriendGrouping() {
//     return _groupingInfos["friend"].keys().toList();
// }

bool ContactPage::loadCacheContact(const QList<FriendshipData> &caches)
{
    for (auto &it : caches) {
        bool isGroup = (it.friendType==2);
        if (!isGroup) {
               UserBaseInfoData uInfo;
               uInfo.ssid=it.friendSSID;
               uInfo.username=it.friendName;
               addFriendGrouping(it.groupingName);
               addContactInfo(it.groupingName,{uInfo,{},{},it.status,false});
        }
        else {
                //status是群成员
                QStringList memberList =it.status.split('|');
                MsgCombineData info;
                QList<GroupMemberInfoData>memberInfo;
                info.isGroup=true;
                info.groupBaseInfo.ssidGroup=it.friendSSID;
                info.groupBaseInfo.createSSID=memberList[0].toInt();
                info.groupBaseInfo.groupName=it.friendName;
                //处理群成员数据
                for(int i=0;i<memberList.size();i++)
                {
                    GroupMemberInfoData data;
                    data.ssidGroup=it.friendSSID;
                    data.ssidMember=memberList[i].toInt();
                    memberInfo.append(data);
                }
                info.groupMemberInfo=memberInfo;
                addContactInfo(it.groupingName,info);
        }
    }
    return true;
}

//工厂函数
ElaTreeView * ContactPage::getFriendTreeView() {
    auto cpObj = new ElaTreeView();
    ContactModel* copiedModel = _friendModel->deepCopy(this);
    cpObj->header()->setVisible(false);
    cpObj->setModel(copiedModel);
    cpObj->setItemDelegate(new ContactDelegate);
    cpObj->setAcceptDrops(true);
    cpObj->setDragEnabled(true);
    cpObj->setDragDropMode(QAbstractItemView::InternalMove);
    cpObj->setSelectionMode(QAbstractItemView::SingleSelection);
    cpObj->setDropIndicatorShown(true);
    cpObj->setEditTriggers(QTreeView::NoEditTriggers);

    // double-clicked add msg card and change to msg page
    connect(cpObj,&QTreeView::doubleClicked,[=](const QModelIndex &index) {
        if (index.parent().isValid()) {
            QString clickedSSID = index.data(ContactDelegate::SSIDRole).toString();
            if (clickedSSID.isEmpty()) return;
            emit sigTriggerAddToCreateGroupList(_ssidToCardInfoHash.value(clickedSSID).userBaseInfo);
        }
    });

    return cpObj;
}

MsgCombineData ContactPage::getCardInfo(const QString &ssid)
{
    return _ssidToCardInfoHash[ssid];
}

//设置成MsgCombineData就可以既对好友进行操作 也可以对群进行操作
void ContactPage::addContactInfo(const QString& groupingName,const MsgCombineData &info) {
    if (!info.isGroup) {
        if (!_groupingInfos["friend"].contains(groupingName)) {
            addFriendGrouping(groupingName);
        }
        QString avatarPath;
        if (info.userBaseInfo.avatarPath.isEmpty() || info.userBaseInfo.avatarPath == "-1") {
            avatarPath = ":/include/Image/Cirno.jpg";
        }else {
            avatarPath = info.userBaseInfo.avatarPath;
        }

        /*
        从代码分析可以看出，这种"先删除后添加"的模式主要用于：

          1. 防止重复添加：在 addGroupingItem 之前先调用 delGroupingItem，确保同一个联系人不会在同一个分组中重复出现
          2. 信息更新：当联系人信息发生变化时（如头像路径、用户名、状态等），先删除旧的条目，再添加新的条目来确保显示的是最
          新信息
          3. 统一处理逻辑：无论是新增联系人还是更新现有联系人，都使用同一个函数
          addContactInfo，通过先删除后添加的方式来统一处理

          在 contactmodel.cpp:64-82 中，delGroupingItem 函数会查找并删除匹配的条目，如果找不到匹配的条目也不会报错，所以这
          种"先删除后添加"的模式是安全的，可以同时处理新增和更新两种情况。
        */
        _friendModel->delGroupingItem(groupingName,{
                                                        QString::number(info.userBaseInfo.ssid),info.userBaseInfo.username,info.content,avatarPath
                                                    });
        _friendModel->addGroupingItem(groupingName,{
                                                        QString::number(info.userBaseInfo.ssid),info.userBaseInfo.username,info.content,avatarPath
                                                    });
        _groupingInfos["friend"][groupingName].append(info);
        _ssidToCardInfoHash[QString::number(info.userBaseInfo.ssid)] = info;
    }else {
        QString avatarPath;
        if (info.userBaseInfo.avatarPath.isEmpty() || info.userBaseInfo.avatarPath == "-1") {
            avatarPath = ":/include/Image/Cirno.jpg";
        }else {
            avatarPath = info.userBaseInfo.avatarPath;
        }
        //content对应的是群成员
        _groupModel->delGroupingItem(groupingName,{
                                                       QString::number(info.groupBaseInfo.ssidGroup),info.groupBaseInfo.groupName,info.content,avatarPath
                                                   });
        _groupModel->addGroupingItem(groupingName,{
                                                       QString::number(info.groupBaseInfo.ssidGroup),info.groupBaseInfo.groupName,info.content,avatarPath
                                                   });
        _groupingInfos["group"][groupingName].append(info);
        _ssidToCardInfoHash[QString::number(info.groupBaseInfo.ssidGroup)] = info;
    }
}

ContactPage::ContactPage(QWidget *parent) : ElaScrollPage(parent) {
    initWindow();

    initEdgeLayout();

    initContent();

    initConnectFunc();

    // _friendModel->addGrouping("303");
    // _friendModel->addGroupingItem("303",{"1","阿旺","离线",":/include/Image/Cirno.jpg"});
    // addContactInfo("303",{{2035,"张毅博",""},{},{},"",false});
    // addContactInfo("303",{{20357,"胡益磊",""},{},{},"",false});
}
ContactPage::~ContactPage() {
    _noticePage->deleteLater();
}

void ContactPage::initWindow() {
    _centralWidget         = new QWidget(this);
    _centralWidLayout      = new QVBoxLayout;
    _friendNoticeButton    = new ElaToolButton(this);
    _groupNoticeButton     = new ElaToolButton(this);
    _friendOrGroupPivot    = new ElaPivot(this);
    _friendTree            = new ElaTreeView(this);
    _groupTree             = new ElaTreeView(this);
    _noticePage            = new NoticePage();
    _scrollTreeLayout      = new QVBoxLayout;
}

void ContactPage::initEdgeLayout() {
    _centralWidLayout->addWidget(_friendNoticeButton);
    _centralWidLayout->addWidget(_groupNoticeButton);
    _centralWidLayout->addWidget(_friendOrGroupPivot);
    _centralWidLayout->setContentsMargins(0,0,0,0);

    _scrollTreeLayout->addWidget(_friendTree);
    _scrollTreeLayout->addWidget(_groupTree);
    _scrollTreeLayout->setContentsMargins(0,0,0,0);

    _centralWidLayout->addItem(_scrollTreeLayout);

    _centralWidget->setLayout(_centralWidLayout);
    _centralWidget->setContentsMargins(0,0,20,0);
    addCentralWidget(_centralWidget,true,true,0);
}

void ContactPage::initContent() {
    setTitleVisible(false);

    _friendNoticeButton->setElaIcon(ElaIconType::AngleRight);
    _friendNoticeButton->setText("好友通知");
    _friendNoticeButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _friendNoticeButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _friendNoticeButton->setStyleSheet("ElaToolButton");
    _friendNoticeButton->setFixedHeight(40);

    _groupNoticeButton->setElaIcon(ElaIconType::AngleRight);
    _groupNoticeButton->setText("群通知");
    _groupNoticeButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _groupNoticeButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _groupNoticeButton->setStyleSheet("ElaToolButton");
    _groupNoticeButton->setFixedHeight(40);

    _friendOrGroupPivot->setTextPixelSize(18);
    _friendOrGroupPivot->setPivotSpacing(10);
    _friendOrGroupPivot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    _friendOrGroupPivot->appendPivot("好友");
    _friendOrGroupPivot->appendPivot("群组");
    _friendOrGroupPivot->setPivotSpacing(150);
    _friendOrGroupPivot->setFixedHeight(40);
    _friendOrGroupPivot->setCurrentIndex(0);

    // init model
    // grouping
    _friendModel = new ContactModel(this);
    _groupModel = new ContactModel(this);
    _groupingInfos["friend"] = {};
    _groupingInfos["group"] = {};
    ContactDelegate * fDelegate = new ContactDelegate(this);
    ContactDelegate * gDelegate = new ContactDelegate(this);
    gDelegate->setGroupFlag();

    _groupModel->addGrouping("未命名群聊");
    _groupModel->addGrouping("我创建的群聊");
    _groupModel->addGrouping("我管理的群聊");
    _groupModel->addGrouping("我加入的群聊");

    _friendTree->setModel(_friendModel);
    _friendTree->setAcceptDrops(true);
    _friendTree->setDragEnabled(true);
    _friendTree->setDragDropMode(QAbstractItemView::InternalMove);
    _friendTree->setSelectionMode(QAbstractItemView::SingleSelection);
    _friendTree->setDropIndicatorShown(true);
    _friendTree->setItemDelegate(fDelegate);
    //禁止用户编辑
    _friendTree->setEditTriggers(QTreeView::NoEditTriggers);

    _groupTree->hide();
    _groupTree->setModel(_groupModel);
    _groupTree->setAcceptDrops(true);
    _groupTree->setDragEnabled(true);
    _groupTree->setDragDropMode(QAbstractItemView::InternalMove);
    _groupTree->setSelectionMode(QAbstractItemView::SingleSelection);
    _groupTree->setDropIndicatorShown(true);
    _groupTree->setItemDelegate(gDelegate);
    _groupTree->setEditTriggers(QTreeView::NoEditTriggers);

    _groupTree->setAutoFillBackground(false);
    _groupTree->viewport()->setAutoFillBackground(false);
    _friendTree->setAutoFillBackground(false);
    _friendTree->viewport()->setAutoFillBackground(false);

    //添加分组信号
    connect(fDelegate, &ContactDelegate::sigAddGrouping,this,[=](const QString& groupingName) {
        _friendModel->addGrouping(groupingName);
    });
}

void ContactPage::initConnectFunc() {
    // Pivot change
    connect(_friendOrGroupPivot,&ElaPivot::pivotClicked,[=](int index){
        // friend or group
        if(index == 0){
            _groupTree->hide();
            _friendTree->show();
        }else{
            _friendTree->hide();
            _groupTree->show();
        }
    });

    // double-clicked add msg card and change to msg page
    connect(_friendTree,&QTreeView::doubleClicked,[=](const QModelIndex &index) {
        if (index.parent().isValid()) {
            QString clickedSSID = index.data(ContactDelegate::SSIDRole).toString();
            if (clickedSSID.isEmpty()) return;
            emit sigTriggerAddMsgCard(_ssidToCardInfoHash.value(clickedSSID));
        }
    });
    connect(_groupTree,&QTreeView::doubleClicked,[=](const QModelIndex &index) {
        if (index.parent().isValid()) {
            QString clickedSSID = index.data(ContactDelegate::SSIDRole).toString();
            if (clickedSSID.isEmpty()) return;
            emit sigTriggerAddMsgCard(_ssidToCardInfoHash.value(clickedSSID));
        }
    });

    connect(_noticePage,&NoticePage::sigHideArchPageMaskEffect,this,&ContactPage::sigHideArchPageMaskEffect);

    connect(_friendNoticeButton,&ElaToolButton::clicked,[=]() {
        emit sigShowArchPageMaskEffect();
        _noticePage->show();
    });

    connect(this, &ContactPage::sigCommunicateRequestBySSID,this,[=](const QString& ssid) {
        emit sigTriggerAddMsgCard(_ssidToCardInfoHash.value(ssid));
    });

    connect(_friendModel, &ContactModel::sigUpdateFriendshipGrouping, this, &ContactPage::sigUpdateFriendshipGrouping);

    // connect(this,&ContactPage::sigAddMakeFriendRecord,_noticePage,&NoticePage::sltMakeFriendRecord);
    // connect(this,&ContactPage::sigAddJoinGroupRecord,_noticePage,&NoticePage::sltJoinGroupRecord);

    //有新用户加入群聊
    connect(ClientRequestHandler::getInstance(),&ClientRequestHandler::newMemberJoinResponse,this,[this](const QJsonObject& obj){
        qint32 gid=obj["gid"].toInt();
        qint32 uid=obj["uid"].toInt();
        QString groupName=obj["groupname"].toString();
        GroupMemberInfoData data;
        data.ssidGroup=gid;
        data.ssidMember=uid;
        _ssidToCardInfoHash[groupName].groupMemberInfo.append(data);
    });
}
