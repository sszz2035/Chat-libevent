#ifndef LandPage_H
#define LandPage_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QPoint>
#include<ElaComboBox.h>
#include<ElaCheckBox.h>
#include<ElaLineEdit.h>
#include<ElaMessageBar.h>
#include "registerpage.h"
#include<utils/net-work/clientconserver.h>

class LandPage : public QWidget
{
    Q_OBJECT

public:
    static LandPage* getInstance();

    static void destroyInstance();

    void isFreezeSignInBtn(bool enable);

    void login_handler(QJsonObject& obj);

private slots:
    void closeWindow();
    void openRegisterPage();
    void loginButtonClicked();
private:
    void initUI();
    void setupLayout();
    void applyStyles();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    explicit LandPage(QWidget *parent = nullptr);
    LandPage(const LandPage&);
    ~LandPage(){}
    LandPage& operator=(const LandPage&);

    // UI Components
    QGridLayout *mainLayout;
    QHBoxLayout *bottomLayout;

    QLabel *avatarLabel;
    ElaComboBox *usernameCombo;
    ElaLineEdit *passwordEdit;
    ElaCheckBox *showPasswordCheck;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QPushButton *forgotPasswordButton;
    QPushButton *closeButton;
    ElaCheckBox* agreeTermsCheck;

    // Background
    QLabel *backgroundLabel;
    QPixmap backgroundPixmap;

    // Mouse drag
    bool isDragging;
    QPoint dragPosition;
        
    static LandPage* instance;
};

#endif // LandPage_H
