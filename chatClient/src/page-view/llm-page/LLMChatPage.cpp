//
// Created by YourName on 2025/12/03.
//

#include "LLMChatPage.h"
#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QScrollBar>
#include <QBuffer>
#include <QDebug>
#include <QKeyEvent>

LLMChatPage* LLMChatPage::_instance = nullptr;
std::mutex LLMChatPageMutex;

LLMChatPage* LLMChatPage::getInstance() {
    if (_instance == nullptr) {
        std::lock_guard<std::mutex> lock(LLMChatPageMutex);
        if (_instance == nullptr) {
            _instance = new LLMChatPage;
        }
    }
    return _instance;
}

void LLMChatPage::destroyInstance() {
    if (_instance != nullptr) {
        std::lock_guard<std::mutex> lock(LLMChatPageMutex);
        if (_instance != nullptr) {
            delete _instance;
            _instance = nullptr;
        }
    }
}

LLMChatPage::LLMChatPage(QWidget* parent)
    : QWidget(parent)
    , _mainLayout(nullptr)
    , _chatLayout(nullptr)
    , _chatContainer(nullptr)
    , _scrollArea(nullptr)
    , _inputLayout(nullptr)
    , _inputTextEdit(nullptr)
    , _sendButton(nullptr)
    , _clearButton(nullptr)
    , _statusLabel(nullptr)
    , _loadingMovie(nullptr)
    , _loadingLabel(nullptr)
    , _networkManager(nullptr)
    , _currentReply(nullptr)
    , _isStreaming(false)
    , _streamingContentLabel(nullptr)
{
    initWindow();
    initChatArea();
    initInputArea();
    initNetwork();

    // 添加欢迎消息
    ChatMessage welcomeMsg;
    welcomeMsg.role = "assistant";
    welcomeMsg.content = "您好！我是DeepSeek AI助手，有什么可以帮助您的吗？";
    welcomeMsg.isUser = false;
    addMessageToChat(welcomeMsg);
}

LLMChatPage::~LLMChatPage() {
    if (_currentReply) {
        _currentReply->abort();
        _currentReply->deleteLater();
    }
}

void LLMChatPage::initWindow() {
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setWindowTitle("DeepSeek AI Chat");
    resize(900, 700);

    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setContentsMargins(10, 10, 10, 10);
    _mainLayout->setSpacing(10);
}

void LLMChatPage::initChatArea() {
    // 创建滚动区域
    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 聊天容器
    _chatContainer = new QWidget(_scrollArea);
    _chatLayout = new QVBoxLayout(_chatContainer);
    _chatLayout->setContentsMargins(10, 10, 10, 10);
    _chatLayout->setSpacing(15);
    _chatLayout->addStretch();

    _scrollArea->setWidget(_chatContainer);
    _mainLayout->addWidget(_scrollArea, 1);

    // 状态标签
    _statusLabel = new QLabel("就绪", this);
    _statusLabel->setAlignment(Qt::AlignCenter);
    _statusLabel->setStyleSheet("QLabel { color: #888; font-size: 12px; }");
    _mainLayout->addWidget(_statusLabel);
}

void LLMChatPage::initInputArea() {
    _inputLayout = new QHBoxLayout();
    _inputLayout->setContentsMargins(0, 0, 0, 0);
    _inputLayout->setSpacing(10);

    // 输入框
    _inputTextEdit = new QTextEdit(this);
    _inputTextEdit->setMaximumHeight(120);
    _inputTextEdit->setPlaceholderText("请输入您的问题...");
    _inputTextEdit->setStyleSheet(R"(
        QTextEdit {
            border: 2px solid #ddd;
            border-radius: 8px;
            padding: 8px;
            font-size: 14px;
            background-color: white;
        }
        QTextEdit:focus {
            border-color: #4a90e2;
        }
    )");
    // 安装事件过滤器以监听键盘事件
    _inputTextEdit->installEventFilter(this);

    // 发送按钮
    _sendButton = new QPushButton("发送", this);
    _sendButton->setMinimumSize(80, 40);
    _sendButton->setStyleSheet(R"(
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2c5a8a;
        }
        QPushButton:disabled {
            background-color: #ccc;
        }
    )");

    // 清空按钮
    _clearButton = new QPushButton("清空", this);
    _clearButton->setMinimumSize(60, 40);
    _clearButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f5f5f5;
            color: #333;
            border: 1px solid #ddd;
            border-radius: 8px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )");

    _inputLayout->addWidget(_inputTextEdit, 1);
    _inputLayout->addWidget(_sendButton);
    _inputLayout->addWidget(_clearButton);

    _mainLayout->addLayout(_inputLayout);

    // 连接信号
    connect(_sendButton, &QPushButton::clicked, this, &LLMChatPage::onSendMessage);
    connect(_clearButton, &QPushButton::clicked, this, &LLMChatPage::onClearChat);
    connect(_inputTextEdit, &QTextEdit::textChanged, [this]() {
        _sendButton->setEnabled(!_inputTextEdit->toPlainText().trimmed().isEmpty() && !_isStreaming);
    });
}

void LLMChatPage::initNetwork() {
    _networkManager = new QNetworkAccessManager(this);
    _loadingMovie = new QMovie(":/loading.gif");
    _loadingLabel = new QLabel(this);
    _loadingLabel->setMovie(_loadingMovie);
    _loadingLabel->setVisible(false);
}

bool LLMChatPage::eventFilter(QObject* obj, QEvent* event) {
    if (obj == _inputTextEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            // 检查是否按下了Enter键（且未按住Shift）
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                if (!keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                    // 如果Shift未被按住，则发送消息
                    onSendMessage();
                    return true;  // 事件已处理，不再传递给QTextEdit
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void LLMChatPage::onSendMessage() {
    QString text = _inputTextEdit->toPlainText().trimmed();
    if (text.isEmpty() || _isStreaming) {
        return;
    }

    // 添加用户消息
    addUserMessage(text);

    // 清空输入框
    _inputTextEdit->clear();

    // 添加助手消息占位符
    ChatMessage assistantMsg;
    assistantMsg.role = "assistant";
    assistantMsg.content = "";
    assistantMsg.isUser = false;
    int msgIndex = _chatHistory.size();
    _chatHistory.append(assistantMsg);

    // 创建助手消息widget并保存label指针用于流式更新
    _streamingContentLabel = addMessageToChat(assistantMsg);

    // 调用API
    callDeepSeekAPI(_chatHistory, true);
}

void LLMChatPage::addUserMessage(const QString& text) {
    ChatMessage msg;
    msg.role = "user";
    msg.content = text;
    msg.isUser = true;
    _chatHistory.append(msg);
    addMessageToChat(msg);
}

void LLMChatPage::addAssistantMessage(const QString& text) {
    // if (!_chatHistory.isEmpty() && !_chatHistory.last().isUser) {
    //     _chatHistory.last().content = text;
    // }
    ChatMessage msg;
    msg.role="assistant";
    msg.content=text;
    msg.isUser=false;
    _chatHistory.append(msg);
    addMessageToChat(msg);
}

QLabel* LLMChatPage::addMessageToChat(const ChatMessage& message) {
    QWidget* messageWidget = new QWidget(_chatContainer);
    QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(10, 5, 10, 5);
    QLabel* contentLabel = nullptr;

    if (message.isUser) {
        // 用户消息 - 右对齐
        messageLayout->addStretch();
        contentLabel = new QLabel(message.content, this);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet(R"(
            QLabel {
                background-color: #4a90e2;
                color: white;
                border-radius: 12px;
                padding: 10px 15px;
                font-size: 14px;
                max-width: 500px;
            }
        )");
        messageLayout->addWidget(contentLabel);
    } else {
        // 助手消息 - 左对齐
        QLabel* avatarLabel = new QLabel("AI", this);
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setMinimumSize(40, 40);
        avatarLabel->setStyleSheet(R"(
            QLabel {
                background-color: #50c878;
                color: white;
                border-radius: 20px;
                font-weight: bold;
            }
        )");

        contentLabel = new QLabel(message.content, this);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet(R"(
            QLabel {
                background-color: #f0f0f0;
                color: #333;
                border-radius: 12px;
                padding: 10px 15px;
                font-size: 14px;
                max-width: 500px;
            }
        )");

        messageLayout->addWidget(avatarLabel);
        messageLayout->addWidget(contentLabel);
        messageLayout->addStretch();
    }

    _chatLayout->addWidget(messageWidget);

    // 滚动到底部
    QTimer::singleShot(100, [this]() {
        _scrollArea->verticalScrollBar()->setValue(_scrollArea->verticalScrollBar()->maximum());
    });

    return contentLabel;
}

void LLMChatPage::callDeepSeekAPI(const QList<ChatMessage>& messages, bool stream) {
    if (_currentReply) {
        _currentReply->abort();
        _currentReply->deleteLater();
    }

    // 设置状态
    _isStreaming = true;
    _sendButton->setEnabled(false);
    _statusLabel->setText("正在思考...");
    _streamingResponse.clear();

    // 构建JSON请求
    QJsonObject requestObj;
    QJsonArray messagesArray;

    for (const auto& msg : messages) {
        QJsonObject msgObj;
        msgObj["role"] = msg.role;
        msgObj["content"] = msg.content;
        messagesArray.append(msgObj);
    }

    requestObj["model"] = DEEPSEEK_MODEL;
    requestObj["messages"] = messagesArray;
    requestObj["stream"] = stream;
    requestObj["temperature"] = 1;

    QJsonDocument doc(requestObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // 创建请求
    QNetworkRequest request;
    request.setUrl(QUrl(DEEPSEEK_API_URL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(DEEPSEEK_API_KEY).toUtf8());

    // 发送请求
    _currentReply = _networkManager->post(request, data);

    // 连接信号
    connect(_currentReply, &QNetworkReply::readyRead, this, &LLMChatPage::onStreamDataReady);
    connect(_currentReply, &QNetworkReply::finished, this, [this]() {
        onApiResponse(_currentReply);
        _currentReply = nullptr;
    });
    connect(_currentReply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
        _statusLabel->setText(QString("网络错误: %1").arg(code));
        _isStreaming = false;
        _sendButton->setEnabled(!_inputTextEdit->toPlainText().trimmed().isEmpty());
    });
}

void LLMChatPage::onStreamDataReady() {
    if (!_currentReply) return;

    QByteArray data = _currentReply->readAll();
    QString text = QString::fromUtf8(data);

    // 解析SSE数据
    parseSSEData(text);
}

void LLMChatPage::parseSSEData(const QString& data) {
    QStringList lines = data.split("\n");
    for (const QString& line : lines) {
        if (line.startsWith("data:")) {
            QString jsonStr = line.mid(5).trimmed();
            if (jsonStr == "[DONE]") {
                _isStreaming = false;
                _sendButton->setEnabled(!_inputTextEdit->toPlainText().trimmed().isEmpty());
                _statusLabel->setText("完成");
                _streamingContentLabel = nullptr;  // 重置流式内容label
                return;
            }

            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("choices") && obj["choices"].isArray()) {
                    QJsonArray choices = obj["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QJsonObject choice = choices[0].toObject();
                        if (choice.contains("delta") && choice["delta"].isObject()) {
                            QJsonObject delta = choice["delta"].toObject();
                            if (delta.contains("content")) {
                                QString content = delta["content"].toString();
                                _streamingResponse += content;

                                // 更新流式内容label而不是创建新消息
                                if (_streamingContentLabel) {
                                    _streamingContentLabel->setText(_streamingResponse);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void LLMChatPage::onApiResponse(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        _statusLabel->setText(QString("请求失败: %1").arg(reply->errorString()));
        _isStreaming = false;
        _sendButton->setEnabled(!_inputTextEdit->toPlainText().trimmed().isEmpty());
        _streamingContentLabel = nullptr;
        return;
    }

    _statusLabel->setText("完成");
    _isStreaming = false;
    _sendButton->setEnabled(!_inputTextEdit->toPlainText().trimmed().isEmpty());
    _streamingContentLabel = nullptr;
}

void LLMChatPage::onClearChat() {
    _chatHistory.clear();

    // 清除聊天区域（除了stretch）
    QLayoutItem* child;
    while ((child = _chatLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    _chatLayout->addStretch();

    // 添加欢迎消息
    ChatMessage welcomeMsg;
    welcomeMsg.role = "assistant";
    welcomeMsg.content = "您好！我是DeepSeek AI助手，有什么可以帮助您的吗？";
    welcomeMsg.isUser = false;
    addMessageToChat(welcomeMsg);
}
