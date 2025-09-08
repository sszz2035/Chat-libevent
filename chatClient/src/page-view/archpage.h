#ifndef SYNERGYSPOT_ARCHPAGE_H
#define SYNERGYSPOT_ARCHPAGE_H

#include "define.h"
#include "ElaWindow.h"
#include "ElaWidget.h"
class ElaContentDialog;
class ElaStatusBar;
class ElaText;
class ElaToolBar;
class ElaToolButton;
class ElaSuggestBox;
class MessagePage;
class AboutPage;
class ContactPage;
class FileManagerPage;
class SettingsPage;
class AddPage;
class CreateGroupPage;
class SSMaskWidget;
class LoadingDialog;

#define g_pArchPage ArchPage::getInstance()

// #include "help.h"

enum PageName {
    MessagePage,
    ContactPage,
    LLMPage,
    FileManagerPage,
    SettingsPage
};

class ArchPage : public ElaWindow{
    Q_OBJECT
signals:
    void sigJumpOtherPageRequest(PageName pageName);
    void sigTriggerUpdate(UserInfo* info);
public:
    static ArchPage * getInstance();
    static void destroyInstance();
    void setInstanceParent(QObject * parent);
    void setUserInfo(UserInfo *info);
public slots:
    void sltTriggerUpdate(UserInfo* info);

    void sltShowMaskEffect();
    void sltHideMaskEffect();
    void sltShowLoading();
    void sltHideLoading();

private:
    explicit ArchPage(QWidget * parent = nullptr);
    ~ArchPage() override;
protected:
    void initWindow();
    void initEdgeLayout();
    void initContent();
    void initConnectFunc();

    virtual void resizeEvent(QResizeEvent * event) override;
private:
    // ----------------- UI -----------------
    ElaContentDialog * _closeDialog      = nullptr;
    ElaStatusBar     * _statusBar        = nullptr;
    ElaText          * _statusText       = nullptr;
    ElaToolBar       * _toolBar          = nullptr;
    ElaToolButton    * _addButton        = nullptr;
    ElaSuggestBox    * _searchSuggest    = nullptr;

    QAction          * _createAction     = nullptr;
    QAction          * _addAction        = nullptr;

    AddPage          * _addPage          = nullptr;
    CreateGroupPage  * _createGroupPage  = nullptr;

    int                _msgNoticeNum     = 0;
    int                _contactNoticeNum = 0;
    int                _llmAppNum        = 0;
    QString            _aboutKey         = "about";
    QString            _settingsKey      = "settings";
    QString            _fileManagerKey   = "fileManager";
    // ----------------- UI -----------------

    // --------------- BackEnd --------------
    // --------------- BackEnd --------------

    static ArchPage * _obj;
    SSMaskWidget  *_maskWidget;
    LoadingDialog *_loadingDialog;

    UserInfo* userInfo;
};

#endif //SYNERGYSPOT_ARCHPAGE_H
