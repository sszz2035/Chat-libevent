#ifndef CREATEGROUPPAGE_H
#define CREATEGROUPPAGE_H

#include <QWidget>

class ElaTreeView;
class ElaListView;
class ElaToolButton;
class ElaScrollBar;
class SSMaskWidget;
class LoadingDialog;
class QVBoxLayout;
class QHBoxLayout;
class QStandardItemModel;

struct UserBaseInfoData;

class CreateGroupPage : public QWidget{
    Q_OBJECT
public:
    CreateGroupPage();
    ~CreateGroupPage();
signals:
    void sigHideArchPageMaskEffect();
    void sigAddBtnClicked(const QString& ssid, bool isGroup);
public slots:
    void sltShowMaskEffect();
    void sltHideMaskEffect();
    void sltShowLoading();
    void sltHideLoading();

    void sltAddToCreateGroupList(const UserBaseInfoData &dto);

protected:
    void initWindow();
    void initEdgeLayout();
    void initContent();
    void initConnectFunc();
    void paintEvent(QPaintEvent *event) override;
private:
    // ----------------- UI -----------------
    QVBoxLayout                     * _mainLayout       = nullptr;
    QHBoxLayout                     * _selectedLayout   = nullptr;
    QHBoxLayout                     * _buttonLayout     = nullptr;
    ElaListView                     * _selectedList     = nullptr;
    ElaScrollBar                    * _listBar          = nullptr;
    ElaToolButton                   * _confirmBtn       = nullptr;
    ElaToolButton                   * _closeBtn         = nullptr;
    // ----------------- UI -----------------

    // --------------- BackEnd --------------
    QStandardItemModel              * _dataModel        = nullptr;
    //key:用户ssid val:用户信息
    QHash<qint32,UserBaseInfoData>    _toAddUserHash;
    // --------------- BackEnd --------------

    SSMaskWidget  *_maskWidget;
    LoadingDialog *_loadingDialog;
};



#endif //CREATEGROUPPAGE_H
