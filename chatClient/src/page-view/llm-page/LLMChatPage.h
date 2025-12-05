//
// Created by YourName on 2025/12/03.
//

#ifndef LLMCHATPAGE_H
#define LLMCHATPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QMovie>
#include<mutex>

#ifdef _WIN32
    #ifdef LLMCHAT_PAGE_EXPORTS
        #define LLMCHAT_API __declspec(dllexport)
    #else
        #define LLMCHAT_API __declspec(dllimport)
    #endif
#elif defined __linux__
    #ifdef LLMCHAT_PAGE_EXPORTS
        #define LLMCHAT_API __attribute__((visibility("default")))
    #else
        #define LLMCHAT_API
    #endif
#endif

#define g_pLLMChatPage LLMChatPage::getInstance()

// DeepSeek API配置
#define DEEPSEEK_API_KEY "sk-3792821f6c8d4d119d75d6838850ba56"  // 替换为您的API Key
#define DEEPSEEK_API_URL "https://api.deepseek.com/chat/completions"
#define DEEPSEEK_MODEL "deepseek-chat"

class LLMCHAT_API LLMChatPage : public QWidget {
    Q_OBJECT
public:
    static LLMChatPage* getInstance();
    static void destroyInstance();

    // 消息结构
    struct ChatMessage {
        QString role;      // "user", "assistant", "system"
        QString content;
        bool isUser;
    };

private:
    explicit LLMChatPage(QWidget* parent = nullptr);
    ~LLMChatPage();

protected:
    void initWindow();
    void initChatArea();
    void initInputArea();
    void initNetwork();
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSendMessage();
    void onApiResponse(QNetworkReply* reply);
    void onStreamDataReady();
    void onClearChat();

private:
    QLabel* addMessageToChat(const ChatMessage& message);
    void addUserMessage(const QString& text);
    void addAssistantMessage(const QString& text);
    void callDeepSeekAPI(const QList<ChatMessage>& messages, bool stream = true);
    void parseSSEData(const QString& data);
    QString formatMessageForDisplay(const QString& role, const QString& content);

    // UI组件
    QVBoxLayout* _mainLayout;
    QVBoxLayout* _chatLayout;
    QWidget* _chatContainer;
    QScrollArea* _scrollArea;
    QHBoxLayout* _inputLayout;
    QTextEdit* _inputTextEdit;
    QPushButton* _sendButton;
    QPushButton* _clearButton;
    QLabel* _statusLabel;
    QMovie* _loadingMovie;
    QLabel* _loadingLabel;

    // 网络相关
    QNetworkAccessManager* _networkManager;
    QNetworkReply* _currentReply;
    QString _streamingResponse;
    bool _isStreaming;

    // 聊天历史
    QList<ChatMessage> _chatHistory;

    // 流式输出相关
    QLabel* _streamingContentLabel;

    static LLMChatPage* _instance;
};

#endif // LLMCHATPAGE_H
