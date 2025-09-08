#ifndef RegisterPage_H
#define RegisterPage_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QPoint>
#include<QJsonObject>
#include<utils/net-work/clientconserver.h>
#include<ElaMessageBar.h>
#include <ElaLineEdit.h>
#include <ElaCheckBox.h>
#include <ElaComboBox.h>

class RegisterPage : public QWidget
{
    Q_OBJECT

public:
    static RegisterPage* getInstance();

    static void destroyInstance();

    void register_handler(QJsonObject& obj);
public slots:
    void closeWindow();
    void returnToLogin();
    void registerButtonClicked();
signals:
    void backToLoginRequested();

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
    explicit RegisterPage(QWidget *parent = nullptr);
    RegisterPage(const RegisterPage&);
    ~RegisterPage(){}
    RegisterPage& operator=(const RegisterPage&);
    // UI Components
    QGridLayout *mainLayout;
    QHBoxLayout *bottomLayout;

    QLabel *titleLabel;
    // QLabel *avatarLabel;
    ElaLineEdit *usernameEdit;
    ElaLineEdit *emailEdit;
    ElaLineEdit *passwordEdit;
    ElaLineEdit *confirmPasswordEdit;
    ElaComboBox *securityQuestionCombo;
    ElaLineEdit *securityAnswerEdit;
    ElaCheckBox *agreeTermsCheck;
    ElaCheckBox *showPasswordCheck;
    QPushButton *registerButton;
    QPushButton *backToLoginButton;
    QPushButton *closeButton;

    // Background
    QPixmap backgroundPixmap;

    // Mouse drag
    bool isDragging;
    QPoint dragPosition;
    
    static RegisterPage* instance;
};

#endif // RegisterPage_H
