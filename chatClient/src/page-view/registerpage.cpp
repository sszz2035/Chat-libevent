#include "registerpage.h"
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

RegisterPage* RegisterPage::instance = nullptr;

RegisterPage::RegisterPage(QWidget *parent)
    : QWidget(parent), isDragging(false)
{
    initUI();
    setupLayout();
    applyStyles();

    setFixedSize(380, 520);
    setWindowTitle("注册账号");

    // Set frameless window
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

RegisterPage* RegisterPage::getInstance()
{
    if (!instance) {
        instance = new RegisterPage();
    }
    return instance;
}

void RegisterPage::destroyInstance()
{
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

void RegisterPage::register_handler(QJsonObject &obj)
{
    if(obj["result"]=="user_exist")
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","用户已存在",1000,this);
    }
    else if(obj["result"]=="success")
    {
        ElaMessageBar::success(ElaMessageBarType::Top,"✅","注册成功",1000,this);
    }
}


void RegisterPage::initUI()
{
    // 创建主布局和底部布局
    mainLayout = new QGridLayout(this);
    bottomLayout = new QHBoxLayout();

    // 创建标题标签
    titleLabel = new QLabel("创建新账号", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    // 创建头像标签
    // size_t labelSize = 70;
    // avatarLabel = new QLabel(this);
    // avatarLabel->setFixedSize(labelSize, labelSize);
    // avatarLabel->setScaledContents(true);
    
    // 设置头像图片
    // QPixmap avatarPixmap(":/images/default_avatar.jpeg");
    
    // 如果为空创建默认头像
    // if (avatarPixmap.isNull()) {
    //     avatarPixmap = QPixmap(labelSize, labelSize);
    //     avatarPixmap.fill(Qt::transparent);
    //     QPainter painter(&avatarPixmap);
    //     painter.setRenderHint(QPainter::Antialiasing);
    //     painter.setBrush(QColor(200, 200, 200));
    //     painter.setPen(Qt::NoPen);
    //     painter.drawEllipse(0, 0, labelSize, labelSize);
    // }
    // avatarLabel->setPixmap(avatarPixmap.scaled(labelSize, labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    // 创建用户名输入框
    usernameEdit = new ElaLineEdit(this);
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setFixedSize(300, 35);

    // 创建邮箱输入框
    emailEdit = new ElaLineEdit(this);
    emailEdit->setPlaceholderText("请输入邮箱地址");
    emailEdit->setFixedSize(300, 35);

    // 创建密码输入框
    passwordEdit = new ElaLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码（至少9位）");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedSize(300, 35);

    // 创建确认密码输入框
    confirmPasswordEdit = new ElaLineEdit(this);
    confirmPasswordEdit->setPlaceholderText("请确认密码");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setFixedSize(300, 35);

    // 创建密保问题下拉框
    securityQuestionCombo = new ElaComboBox(this);
    securityQuestionCombo->setPlaceholderText("请选择密保问题");
    securityQuestionCombo->addItem("您的生日是？");
    securityQuestionCombo->addItem("您的母亲姓名是？");
    securityQuestionCombo->addItem("您的毕业学校是？");
    securityQuestionCombo->addItem("您的爱好是？");
    securityQuestionCombo->addItem("您的宠物名字是？");
    securityQuestionCombo->setFixedSize(300, 35);

    // 创建密保答案输入框
    securityAnswerEdit = new ElaLineEdit(this);
    securityAnswerEdit->setPlaceholderText("请输入密保答案");
    securityAnswerEdit->setFixedSize(300, 35);

    // 创建显示密码复选框
    showPasswordCheck = new ElaCheckBox("显示密码", this);

    // 创建同意条款复选框
    agreeTermsCheck = new ElaCheckBox("我已阅读并同意用户协议和隐私政策", this);

    // 创建注册按钮
    registerButton = new QPushButton("立即注册", this);
    registerButton->setFixedSize(300, 40);

    // 创建返回登录按钮
    backToLoginButton = new QPushButton("返回登录", this);
    backToLoginButton->setFlat(true);

    // 创建关闭按钮
    closeButton = new QPushButton("×", this);
    closeButton->setFixedSize(30, 30);
    closeButton->setObjectName("closeButton");
}

void RegisterPage::setupLayout()
{
    // 添加控件到主布局
    mainLayout->addWidget(titleLabel, 0, 0, 1, 1, Qt::AlignCenter);
    // mainLayout->addWidget(avatarLabel, 1, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(usernameEdit, 2, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(emailEdit, 3, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(passwordEdit, 4, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(confirmPasswordEdit, 5, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(securityQuestionCombo, 6, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(securityAnswerEdit, 7, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(showPasswordCheck, 8, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(agreeTermsCheck, 9, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(registerButton, 10, 0, 1, 1, Qt::AlignCenter);

    // 底部布局
    bottomLayout->addWidget(backToLoginButton);
    bottomLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(bottomLayout, 11, 0, 1, 1, Qt::AlignCenter);

    // 设置间距
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setVerticalSpacing(10);

    setLayout(mainLayout);

    // 移动关闭按钮到右方
    closeButton->move(width() - 35, 8);

    // 连接信号槽
    connect(closeButton, &QPushButton::clicked, this, &RegisterPage::closeWindow);
    connect(backToLoginButton, &QPushButton::clicked, this, &RegisterPage::returnToLogin);
    connect(showPasswordCheck, &ElaCheckBox::toggled, this, [this](bool checked) {
        passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        confirmPasswordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    connect(registerButton,&QPushButton::clicked,this,&RegisterPage::registerButtonClicked);
}

void RegisterPage::applyStyles()
{
    // 创建圆形头像
    // QPixmap avatarPixmap = avatarLabel->pixmap(Qt::ReturnByValue);
    // if (!avatarPixmap.isNull()) {
    //     QPixmap circularPixmap(70, 70);
    //     circularPixmap.fill(Qt::transparent);

    //     QPainter painter(&circularPixmap);
    //     painter.setRenderHint(QPainter::Antialiasing);
    //     painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //     QPainterPath path;
    //     path.addEllipse(0, 0, 70, 70);
    //     painter.setClipPath(path);

    //     painter.drawPixmap(0, 0, avatarPixmap.scaled(70, 70, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    //     avatarLabel->setPixmap(circularPixmap);
    // }

    // 头像样式
    // avatarLabel->setStyleSheet("QLabel { "
    //                            "background: transparent; "
    //                            "border: none; "
    //                            "}");

    // 头像阴影效果
    // QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    // shadowEffect->setBlurRadius(15);
    // shadowEffect->setColor(QColor(0, 0, 0, 100));
    // shadowEffect->setOffset(0, 4);
    // avatarLabel->setGraphicsEffect(shadowEffect);

    // 标题样式
    titleLabel->setStyleSheet("QLabel { "
                              "font-size: 20px; "
                              "font-weight: bold; "
                              "color: #333; "
                              "}");

    // 复选框样式
    showPasswordCheck->setStyleSheet("ElaCheckBox { "
                                     "font-size: 13px; "
                                     "}");

    agreeTermsCheck->setStyleSheet("ElaCheckBox { "
                                  "font-size: 12px; "
                                  "}");

    // 注册按钮样式
    registerButton->setStyleSheet("QPushButton { "
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

    // 返回登录按钮样式
    backToLoginButton->setStyleSheet("QPushButton { "
                                    "color: #12B7F5; "
                                    "border: none; "
                                    "font-size: 12px; "
                                    "text-decoration: none; "
                                    "}"
                                    "QPushButton:hover { "
                                    "color: #0FA0E0; "
                                    "}");

    // 关闭按钮样式
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
}

void RegisterPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角矩形背景
    QRectF rect(0, 0, width(), height());
    painter.setBrush(QColor(224, 240, 255)); // #E0F0FF 背景
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 15, 15);

    // 绘制边框
    painter.setPen(QPen(QColor(180, 200, 220), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 15, 15);

    // 确保关闭按钮在最上层
    if (closeButton) {
        closeButton->raise();
    }
}

void RegisterPage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void RegisterPage::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && event->buttons() == Qt::LeftButton) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void RegisterPage::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
    }
}

void RegisterPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 重新定位关闭按钮
    if (closeButton) {
        closeButton->move(width() - 35, 8);
        closeButton->raise();
    }
}

void RegisterPage::closeWindow()
{
    close();
}

void RegisterPage::returnToLogin()
{
    emit backToLoginRequested();
    hide();
}

void RegisterPage::registerButtonClicked()
{
    QString userName=usernameEdit->text();
    QString passWord=passwordEdit->text();
    if(!agreeTermsCheck->isChecked())
    {
        ElaMessageBar::error(ElaMessageBarType::Top,"⚠","请同意隐私和服务条款！",1000,this);
        return;
    }
    if(userName.isEmpty())
    {
        ElaMessageBar::error(ElaMessageBarType::TopRight,"⚠","请输入用户名！",1000,this);
        return;
    }
    if(passWord.isEmpty()||confirmPasswordEdit->text().isEmpty())
    {
        ElaMessageBar::error(ElaMessageBarType::Right,"⚠","请输入密码！",1000,this);
        return;
    }
    if(passWord!=confirmPasswordEdit->text())
    {
        ElaMessageBar::error(ElaMessageBarType::Right,"⚠","两次输入的密码不一致！",1000,this);
        return;
    }
    QJsonObject obj;
    obj["cmd"]="register";
    obj["username"]=userName;
    obj["password"]=passWord;

    ClientConServer::getInstance()->clinet_write_data(obj);
}
