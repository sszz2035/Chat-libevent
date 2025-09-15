#include "LandPage.h"
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QTextStream>
#include <QGraphicsDropShadowEffect>
#include"core/loginverify.h"
#include"page-view/archpage.h"
#include<QTimer>
LandPage* LandPage::instance = nullptr;

LandPage::LandPage(QWidget *parent)
    : QWidget(parent), isDragging(false)
{
    initUI();
    setupLayout();
    applyStyles();

    setFixedSize(325, 440);
    setWindowTitle("登录页面");

    // Set frameless window
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
    connect(this,&LandPage::sigTriggerUpdate,ArchPage::getInstance(),&ArchPage::sltTriggerUpdate);
}

LandPage* LandPage::getInstance()
{
    if (!instance) {
        instance = new LandPage();
    }
    return instance;
}

void LandPage::destroyInstance()
{
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

void LandPage::isFreezeSignInBtn(bool enable)
{
    // if(enable)
    // {
    //     loginButton->setStyleSheet("QPushButton{"
    //                                "background-color: grey;"
    //                                "color: black;"
    //                                "}");
    // }
    // else
    // {
    //     loginButton->setStyleSheet("QPushButton{"
    //                                "background-color: #12B7F5;"
    //                                "color: white;"
    //                                "}");
    // }
    loginButton->setEnabled(!enable);
}

void LandPage::login_handler(QJsonObject &obj)
{
    if(obj.value("result").toString()=="not_exist")
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","用户不存在！",1000,this);    isFreezeSignInBtn(true);
        isFreezeSignInBtn(false);
    }
    else if(obj.value("result").toString()=="password_error")
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","密码错误！",1000,this);
        isFreezeSignInBtn(false);
    }
    else if(obj.value("result").toString()=="success")
    {
        int ssid=obj["uid"].toInt();
        QString friList=obj["friendlist"].toString();
        QString groList=obj["grouplist"].toString();
        UserInfo* info=new UserInfo();
        info->_ssid=ssid;
        info->_name=obj["username"].toString();
        info->_friList=friList;
        info->_groList=groList;
        info->_type=Myself;
        emit(sigTriggerUpdate(info));
        ElaMessageBar::success(ElaMessageBarType::Top,"✅","登录成功！",1000,this);
        QTimer::singleShot(1500, this, [this]() {
            hide();
            ArchPage::getInstance()->show();
            destroyInstance();
        });
    }
}

void LandPage::initUI()
{
    // 创建主布局 底部按钮布局
    mainLayout = new QGridLayout(this);
    bottomLayout = new QHBoxLayout();

    //创建头像标签
    size_t labelSize=76;
    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(labelSize, labelSize);
    avatarLabel->setScaledContents(true);
    //设置头像图片
    QPixmap avatarPixmap(":/images/default_avatar.jpeg");

    //如果为空创建默认头像
    if (avatarPixmap.isNull()) {
        avatarPixmap = QPixmap(labelSize, labelSize);
        avatarPixmap.fill(Qt::transparent);
        QPainter painter(&avatarPixmap);
        painter.setRenderHint(QPainter::Antialiasing);//抗锯齿
        painter.setBrush(QColor(200, 200, 200));
        painter.setPen(Qt::NoPen);//绘制图形时不画边框（轮廓），只绘制填充内容
        painter.drawEllipse(0, 0, labelSize, labelSize);
    }
    avatarLabel->setPixmap(avatarPixmap.scaled(labelSize, labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    //创建用户名comboBox
    usernameCombo = new QComboBox(this);
    usernameCombo->setEditable(true);
    usernameCombo->setPlaceholderText("请输入用户名");
    usernameCombo->setFixedSize(252, 40);

    //创建密码输入框
    passwordEdit = new ElaLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedSize(252, 35);

    //创建显示密码Checkbox
    showPasswordCheck = new ElaCheckBox("显示密码", this);

    //创建用户协议checkbox
    agreeTermsCheck = new ElaCheckBox("我已阅读并同意用户协议和隐私政策", this);

    //创建登录按钮
    loginButton = new QPushButton("登录", this);
    loginButton->setFixedSize(260, 40);
    loginButton->setEnabled(false);//默认冻结按钮


    //创建注册按钮
    registerButton = new QPushButton("注册账号", this);
    registerButton->setFlat(true);

    //创建忘记密码按钮
    forgotPasswordButton = new QPushButton("忘记密码", this);
    forgotPasswordButton->setFlat(true);

    //创建关闭按钮
    closeButton = new QPushButton("×", this);
    closeButton->setFixedSize(30, 30);
    closeButton->setObjectName("closeButton");

    //创建背景
    backgroundLabel = new QLabel(this);
    backgroundLabel->setGeometry(0, 0, width(), height());
    backgroundLabel->lower();
}

void LandPage::setupLayout()
{
    // 添加widget到主布局
    mainLayout->addWidget(avatarLabel, 0, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(usernameCombo, 1, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(passwordEdit, 2, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(showPasswordCheck, 3, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(agreeTermsCheck, 4, 0, 1, 1,Qt::AlignCenter);
    mainLayout->addWidget(loginButton, 5, 0, 1, 1, Qt::AlignCenter);

    // 底层布局
    bottomLayout->addWidget(registerButton);
    bottomLayout->addWidget(forgotPasswordButton);
    bottomLayout->setSpacing(20);
    mainLayout->addLayout(bottomLayout, 6, 0, 1, 1, Qt::AlignCenter);

    //设置间距
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(35, 45, 35, 30);

    //各行控件之间的垂直间距为 20 像素。
    mainLayout->setVerticalSpacing(20);

    setLayout(mainLayout);

    //移动关闭按钮到右方
    closeButton->move(width() - 35, 8);

    // Connect close button signal
    connect(closeButton, &QPushButton::clicked, this, &LandPage::closeWindow);
    
    // Connect register button signal
    connect(registerButton, &QPushButton::clicked, this, &LandPage::openRegisterPage);

    //显示密码连接信号
    connect(showPasswordCheck, &ElaCheckBox::toggled, this, [this](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    //登录按键连接信号
    connect(loginButton,&QPushButton::clicked,this,&LandPage::loginButtonClicked);
}

void LandPage::applyStyles()
{
    // Window style - now handled by paintEvent for rounded corners

    // Create circular avatar mask
    QPixmap avatarPixmap = avatarLabel->pixmap(Qt::ReturnByValue);
    if (!avatarPixmap.isNull()) {
        QPixmap circularPixmap(70, 70);
        circularPixmap.fill(Qt::transparent);

        QPainter painter(&circularPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        //创建一个圆形的路径（Ellipse），用于对绘制图像进行剪裁。
        QPainterPath path;
        path.addEllipse(0, 0, 70, 70);
        //超出路径部分会被裁剪
        painter.setClipPath(path);

        // Draw the avatar image
        painter.drawPixmap(0, 0, avatarPixmap.scaled(70, 70, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

        avatarLabel->setPixmap(circularPixmap);
    }

    // Avatar style
    avatarLabel->setStyleSheet("QLabel { "
                               "background: transparent; "
                               "border: none; "
                               "}");

    //为一个 avatarLabel 添加一个阴影效果
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    shadowEffect->setOffset(0, 4);
    avatarLabel->setGraphicsEffect(shadowEffect);

    // Show password checkbox style
    showPasswordCheck->setStyleSheet("ElaCheckBox { "
                                     "font-size: 14px; "
                                     "}");


    // Login button style
    loginButton->setStyleSheet("QPushButton { "
                               "background-color: #12B7F5; "
                               "color: white; "
                               "border: none; "
                               "border-radius: 8px; "
                               "font-size: 16px; "
                               "font-weight: bold; "
                               "padding: 8px; "
                               "}"
                               "QPushButton:hover { "
                               "background-color: #0FA0E0; "
                               "}"
                               "QPushButton:pressed { "
                               "background-color: #0D8AC8; "
                               "}");

    // Bottom buttons style
    registerButton->setStyleSheet("QPushButton { "
                                  "color: #12B7F5; "
                                  "border: none; "
                                  "font-size: 12px; "
                                  "text-decoration: none; "
                                  "}"
                                  "QPushButton:hover { "
                                  "color: #0FA0E0; "
                                  "}");

    forgotPasswordButton->setStyleSheet("QPushButton { "
                                        "color: #12B7F5; "
                                        "border: none; "
                                        "font-size: 12px; "
                                        "text-decoration: none; "
                                        "}"
                                        "QPushButton:hover { "
                                        "color: #0FA0E0; "
                                        "}");

    // Close button style
    closeButton->setStyleSheet("QPushButton { "
                               "background-color: transparent; "
                               "color: #666; "
                               "border: none; "
                               "font-size: 20px; "
                               "font-weight: bold; "
                               "border-radius: 15px; "
                               "}"
                               "QPushButton:hover { "
                               "background-color: rgba(255, 0, 0, 0.1); "
                               "color: #ff4444; "
                               "}"
                               "QPushButton:pressed { "
                               "background-color: rgba(255, 0, 0, 0.2); "
                               "}");

    agreeTermsCheck->setStyleSheet("ElaCheckBox { "
                                   "font-size: 12px; "
                                   "}");
}

//画窗口
void LandPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw rounded rectangle background
    QRectF rect(0, 0, width(), height());
    painter.setBrush(QColor(224, 240, 255)); // #E0F0FF background 背景颜色
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 15, 15);

    // Draw subtle border
    painter.setPen(QPen(QColor(180, 200, 220), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 15, 15);

    // Ensure close button is on top
    if (closeButton) {
        closeButton->raise();
    }
}

void LandPage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void LandPage::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && event->buttons() == Qt::LeftButton) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void LandPage::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
    }
}

void LandPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Reposition close button when window is resized
    if (closeButton) {
        closeButton->move(width() - 35, 8);
        closeButton->raise();
    }
}

void LandPage::closeWindow()
{
    close();
}

void LandPage::openRegisterPage()
{

        // 设置注册界面位置在登录界面附近
        QPoint loginPos = this->pos();
        RegisterPage::getInstance()->move(loginPos.x()-10, loginPos.y()-10);
        
        // 连接注册页面的返回登录信号
        connect(RegisterPage::getInstance(), &RegisterPage::backToLoginRequested, this, [this]() {
            RegisterPage::getInstance()->hide();
            show();
        });
    
    // 隐藏登录页面，显示注册页面
    hide();
    RegisterPage::getInstance()->show();
    RegisterPage::getInstance()->raise(); // 确保窗口在最前面
    RegisterPage::getInstance()->activateWindow(); // 激活窗口
}

void LandPage::loginButtonClicked()
{
    if(!agreeTermsCheck->isChecked())
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","请同意隐私和服务条款！",1000,this);
        return;
    }
    else if(usernameCombo->currentText().isEmpty())
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","请输入用户名",1000,this);
        return;
    }
    else if(passwordEdit->text().isEmpty())
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","请输入密码",1000,this);
        return;
    }
    isFreezeSignInBtn(true);
    LoginVerify login;
    login.setSsid(usernameCombo->currentText().toInt());
    login.setPassWord(passwordEdit->text());
    login.verifyServer();
}
