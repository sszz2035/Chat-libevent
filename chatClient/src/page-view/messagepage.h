#ifndef SYNERGYSPOT_MESSAGEPAGE_H
#define SYNERGYSPOT_MESSAGEPAGE_H

// #include "define.h"

#include <QWidget>
// #include "common-data/CommonData.h"
#include"core/commondata.h"

#ifdef SS_PLATFORM_WINDOWS
#ifdef SS_MESSAGE_PAGE_EXPORTS
#define SS_API __declspec(dllexport)
#else
#define SS_API __declspec(dllimport)
#endif
#elif defined SS_PLATFORM_LINUX
#ifdef SS_MESSAGE_PAGE_EXPORTS
#define SS_API __attribute__((visibility("default")))
#else
#define SS_API
#endif
#endif

class ConversationPage;
class ElaInteractiveCard;
class ElaScrollPage;
class ElaTabWidget;
class ArchPage;
class ElaDockWidget;
class QVBoxLayout;

#define g_pMessagePage MessagePage::getInstance()

class  MessagePage : public QWidget{
    Q_OBJECT
    // grandson class to link card and set content display
    friend class ConversationPage;
public:
    static MessagePage * getInstance();
    static void destroyMessagePage();

    bool loadCacheMsg(QList<MessageContentData> caches);
signals:
    void sigClickedSSIDCardRequest(const qint32& ssid);
public slots:
    // only process card ui logic , don't include interaction with conversation page
    void addMsgCard(const MsgCombineData& info);
private:
    explicit MessagePage(QWidget *parent = nullptr);
    ~MessagePage() override;

    void addMsgContent(const MessageContentData& content);
protected:
    void initConnectFunc();
    void initWindow();
    void initEdgeLayout();
    void initContent();
private:
    // ----------------- UI -----------------
    ElaScrollPage               * _tempMsgList;
    ElaTabWidget                * _conversionWid;
    QVBoxLayout                 * _tempMsgWidLayout;
    QWidget                     * _tempMsgWid;
    // ----------------- UI -----------------

    // --------------- BackEnd --------------
    QMap<ElaInteractiveCard*,MsgCombineData>      _tmpUserMsgList;//卡片对应用户信息容器
    QHash<qint32,int>                           _unreadMsgCount;//统计未读消息数目容器
    QHash<ElaInteractiveCard*,ConversationPage*> _cardLinkPageHash;//卡片对应页面容器
    QHash<qint32,ElaInteractiveCard*>           _ssidLinkCardHash;//用户id对应卡片容器
    QHash<qint32,int>                           _tabSSIDLinkIndex;//用户id对应导航栏序号
    // --------------- BackEnd --------------
    static MessagePage* _messagePage;
};

#endif//SYNERGYSPOT_MESSAGEPAGE_H
