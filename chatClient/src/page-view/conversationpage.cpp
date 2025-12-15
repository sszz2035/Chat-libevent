#include "conversationpage.h"
#include"effect-component/msg-bubble/msgbubblemodel.h"
#include"effect-component/msg-bubble/msgbubbledelegate.h"
#include"effect-component/msg-bubble/msgbubbleview.h"

#include "screenshotpage.h"

#include"emojipickerpage.h"

#include"groupmemberdock.h"

#include "userpage.h"
#include "messagepage.h"
#include "define.h"
#include "ElaToolButton.h"
#include "ElaDockWidget.h"
#include "ElaPushButton.h"
#include "ElaMenu.h"
#include "ElaInteractiveCard.h"
#include "ElaTheme.h"
#include <qpushbutton.h>
#include"utils/get-time/GetCurTime.h"
#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <QGridLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QPainter>
#include <QDateTime>
#include <QListView>
#include <QTimer>
#include <mutex>
#include <QJsonArray>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QBuffer>
#include <QCoreApplication>

#include <core/clientrequesthandler.h>
#include "utils/log/logfile.h"

// 文件类型检测辅助函数
QString detectFileExtension(const QByteArray& fileData, const QString& originalFileName) {
    // 首先尝试从原始文件名提取扩展名
    if (originalFileName.contains('.')) {
        QString ext = originalFileName.mid(originalFileName.lastIndexOf('.'));
        if (!ext.isEmpty()) {
            return ext;
        }
    }

    // 如果没有扩展名，根据文件头检测类型
    if (fileData.size() < 4) {
        return "";
    }

    // 读取前几个字节作为魔数
    quint32 magic = 0;
    if (fileData.size() >= 4) {
        magic = (quint8)fileData[0] << 24 | (quint8)fileData[1] << 16 |
                (quint8)fileData[2] << 8 | (quint8)fileData[3];
    }

    // PDF: %PDF
    if (fileData.startsWith("%PDF")) {
        return ".pdf";
    }
    // ZIP: PK
    if (fileData.startsWith("PK\x03\x04") || fileData.startsWith("PK\x05\x06") || fileData.startsWith("PK\x07\x08")) {
        return ".zip";
    }
    // RAR: Rar!
    if (fileData.startsWith("Rar!")) {
        return ".rar";
    }
    // 7Z: 7z
    if (fileData.startsWith("7z")) {
        return ".7z";
    }
    // GIF: GIF87a or GIF89a
    if (fileData.startsWith("GIF87a") || fileData.startsWith("GIF89a")) {
        return ".gif";
    }
    // PNG: \x89PNG
    if (fileData.startsWith("\x89PNG")) {
        return ".png";
    }
    // JPEG: FF D8 FF
    if (fileData.size() >= 3 && (quint8)fileData[0] == 0xFF && (quint8)fileData[1] == 0xD8 && (quint8)fileData[2] == 0xFF) {
        return ".jpg";
    }
    // BMP: BM
    if (fileData.startsWith("BM")) {
        return ".bmp";
    }
    // TIFF: II* or MM*
    if (fileData.startsWith("II*\x00") || fileData.startsWith("MM\x00*")) {
        return ".tiff";
    }
    // MP3: ID3 or FFB
    if (fileData.startsWith("ID3") || ((quint8)fileData[0] == 0xFF && ((quint8)fileData[1] & 0xF0) == 0xF0)) {
        return ".mp3";
    }
    // MP4: ftyp
    if (fileData.size() >= 4 && fileData[4] == 'f' && fileData[5] == 't' && fileData[6] == 'y' && fileData[7] == 'p') {
        return ".mp4";
    }
    // AVI: RIFF....AVI
    if (fileData.startsWith("RIFF") && fileData.size() >= 12 && fileData[8] == 'A' && fileData[9] == 'V' && fileData[10] == 'I') {
        return ".avi";
    }
    // WAVE: RIFF....WAVE
    if (fileData.startsWith("RIFF") && fileData.size() >= 12 && fileData[8] == 'W' && fileData[9] == 'A' && fileData[10] == 'V' && fileData[11] == 'E') {
        return ".wav";
    }
    // EXE: MZ
    if (fileData.startsWith("MZ")) {
        return ".exe";
    }
    // DLL: PE
    if (fileData.size() >= 2 && (quint8)fileData[0] == 'P' && (quint8)fileData[1] == 'E') {
        return ".dll";
    }
    // DOC: D0CF11E0
    if (fileData.size() >= 4 && (quint8)fileData[0] == 0xD0 && (quint8)fileData[1] == 0xCF && (quint8)fileData[2] == 0x11 && (quint8)fileData[3] == 0xE0) {
        return ".doc";
    }
    // TXT: UTF-8 BOM or plain text
    if (fileData.startsWith("\xEF\xBB\xBF") || fileData.startsWith("{\rtf")) {
        return ".txt";
    }

    return "";
}

SSTextEdit::SSTextEdit(QWidget *parent): QTextEdit(parent) {
    setAcceptDrops(true);
}

void SSTextEdit::insertFromMimeData(const QMimeData *source) {
    if (source->hasImage()) {
        QImage image = qvariant_cast<QImage>(source->imageData());
        insertImage(image);
    } else if (source->hasUrls()) {
        QList<QUrl> urls = source->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QImage image(url.toLocalFile());
                if (!image.isNull()) {
                    insertImage(image);
                }
            }
        }
    } else {
        QTextEdit::insertFromMimeData(source);
    }
}

void SSTextEdit::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasImage() || event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void SSTextEdit::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        insertImage(image);
    } else if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QImage image(url.toLocalFile());
                if (!image.isNull()) {
                    insertImage(image);
                }
            }
        }
    }
}

void SSTextEdit::keyPressEvent(QKeyEvent *event) {
    if (document()->isEmpty() && event->key() == Qt::Key_Backspace) {
        _imagesTmpMap.clear();
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (event->modifiers() & Qt::ControlModifier ) {
            if (_isEnterToSendMsg) {
                insertPlainText("\n");  // Ctrl + Enter 换行
            }else {
                emit sigSendMsgTrigger();  // Ctrl + Enter 触发提交
            }
            event->accept();
        } else {
            if (_isEnterToSendMsg) {
                emit sigSendMsgTrigger();  // 单独 Enter 触发提交
            }else {
                insertPlainText("\n");  // 单独 Enter 换行
            }
            event->accept();
        }
        return ;
    }
    QTextEdit::keyPressEvent(event);
}

void SSTextEdit::insertImage(const QImage &image, double scale) {
    QTextCursor cursor = textCursor();
    QTextDocument *document = this->document();

    int width = static_cast<int>(image.width() * scale);
    int height = static_cast<int>(image.height() * scale);

    // 生成唯一图片名称
    QString imageName = "img_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    document->addResource(QTextDocument::ImageResource, QUrl(imageName), QVariant(image));

    QTextImageFormat imageFormat;
    imageFormat.setName(imageName);
    imageFormat.setWidth(width);
    imageFormat.setHeight(height);
    cursor.insertImage(imageFormat);

    _imagesTmpMap.insert(imageName, image);
}

QMap<QString, QImage>& SSTextEdit::getImageTmpMap() {
    return _imagesTmpMap;
}

void SSTextEdit::setSendMsgStrategy(bool isEnterToSendMsg) {
    _isEnterToSendMsg = isEnterToSendMsg;
}

InputWidget::InputWidget(QWidget *parent) : QWidget(parent)
{
    initWindow();

    initEdgeLayout();

    initContent();

    initConnectFunc();
}

InputWidget::~InputWidget() {
    delete _emojiPickerPage;
    _emojiPickerPage = nullptr;
}

void InputWidget::initWindow() {
    setContentsMargins(0,0,0,0);
    setMaximumHeight(200);
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    _emojiButton         =      new ElaToolButton(this);
    _screenCutButton     =      new ElaToolButton(this);
    _fileButton          =      new ElaToolButton(this);
    _picButton           =      new ElaToolButton(this);
    _voiceMsgButton      =      new ElaToolButton(this);
    _historyMsgButton    =      new ElaToolButton(this);
    _inputEditFrame      =      new SSTextEdit(this);
    _sendButton          =      new QPushButton(this);
    _sendModButton       =      new ElaToolButton(this);
    _sendMod             =      new ElaMenu(this);
    _inputLayout         =      new QGridLayout(this);
    _emojiPickerPage     =      new EmojiPickerPage();
}

void InputWidget::initEdgeLayout() {
    QHBoxLayout * _sendBtnLayout = new QHBoxLayout;
    _sendBtnLayout->setSpacing(5);
    _sendBtnLayout->addStretch();
    _sendBtnLayout->addWidget(_sendButton);
    _sendBtnLayout->addWidget(_sendModButton);
    _sendBtnLayout->setContentsMargins(0,0,0,0);

    _inputEditFrame->setContentsMargins(10,5,10,5);

    // 7 * 12
    _inputLayout->addWidget(_emojiButton,0,0,1,1);
    _inputLayout->addWidget(_screenCutButton,0,1,1,1);
    _inputLayout->addWidget(_fileButton,0,2,1,1);
    _inputLayout->addWidget(_picButton,0,3,1,1);
    _inputLayout->addWidget(_voiceMsgButton,0,4,1,1);
    _inputLayout->addWidget(_historyMsgButton,0,11,1,1);
    _inputLayout->addWidget(_inputEditFrame,1,0,5,12);
    _inputLayout->addItem(_sendBtnLayout,6,0,1,12);
    _inputLayout->setContentsMargins(10,10,0,0);

    this->setLayout(_inputLayout);
}

void InputWidget::initContent() {
    _emojiButton->setElaIcon(ElaIconType::FaceSmile);
    _emojiButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _screenCutButton->setElaIcon(ElaIconType::Scissors);
    _screenCutButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _fileButton->setElaIcon(ElaIconType::FolderArrowUp);
    _fileButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _picButton->setElaIcon(ElaIconType::FolderImage);
    _picButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _voiceMsgButton->setElaIcon(ElaIconType::Microphone);
    _voiceMsgButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _historyMsgButton->setElaIcon(ElaIconType::ClockThree);
    _historyMsgButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    _inputEditFrame->setObjectName(QString::fromUtf8("_inputEditFrame"));
    _inputEditFrame->setStyleSheet("#_inputEditFrame{border:none;background-color:rgb(242,242,242);}");
    _inputEditFrame->viewport()->setAutoFillBackground(true);

    _enterStrategy = _sendMod->addElaIconAction(ElaIconType::CircleCheck,"按 Enter 发送消息");
    _ctrlAndEnterStrategy = _sendMod->addElaIconAction(ElaIconType::Circle,"按 Ctrl + Enter 发送消息");

    _sendModButton->setIsTransparent(false);
    _sendModButton->setMenu(_sendMod);
    _sendModButton->setFixedHeight(30);
    _sendModButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    _sendButton->setText("Send");
    _sendButton->setObjectName(QString::fromUtf8("_sendButton"));
    _sendButton->setStyleSheet("QPushButton{background-color:rgb(  0, 153, 255);color:white;border-radius : 5px;}"
                               "QPushButton:pressed{background-color:rgb(   0, 141, 235);}"
                               "QPushButton:hover{background-color:rgb(  0, 141, 255);}");
    _sendButton->setFixedHeight(_sendModButton->height());
    _sendButton->setFixedWidth(50);
    setObjectName(QString::fromUtf8("_inputEditWid"));
    setStyleSheet("#_inputEditWid {border:none;background-color:rgb(242,242,242);border-bottom-left-radius: 30px;}");

    setMouseTracking(true);
}

void InputWidget::initConnectFunc() {
    connect(_screenCutButton, &ElaPushButton::clicked, this, [=]() {
        ScreenshotPage *screenshotWidget = new ScreenshotPage();
        screenshotWidget->show();
    });

    // 图片选择按钮
    connect(_picButton, &ElaToolButton::clicked, this, [=]() {
        QStringList fileNames = QFileDialog::getOpenFileNames(
            this,
            "选择图片",
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
            "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"
        );
        for (const QString& fileName : fileNames) {
            QImage image(fileName);
            if (!image.isNull()) {
                _inputEditFrame->insertImage(image);
            }
        }
        _inputEditFrame->setFocus();
    });

    // 文件选择按钮
    connect(_fileButton, &ElaToolButton::clicked, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "选择文件",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            "所有文件 (*.*)"
        );
        if (!fileName.isEmpty()) {
            // 发送文件信号
            emit sigFileBtnClicked(fileName);
        }
    });

    connect(_sendButton, &QPushButton::clicked, this, [=]() {
        if (!_inputEditFrame->document()->isEmpty()) {
            emit sigSendBtnClicked(_inputEditFrame->toHtml());
        }
    });

    connect(_emojiButton,&QPushButton::clicked, this, [=]() {
        QPoint globalPos = QCursor::pos();

        globalPos.setY( globalPos.y() - _emojiPickerPage->height());
        _emojiPickerPage->move(globalPos + QPoint{10,10});
        _emojiPickerPage->show();
    });

    connect(_emojiPickerPage, &EmojiPickerPage::sigEmojiSelected, this, [=](const QString &emoji) {
        _inputEditFrame->insertPlainText(emoji);
        _inputEditFrame->setFocus();
    });

    connect(_enterStrategy, &QAction::triggered, this, [=]() {
        _inputEditFrame->setSendMsgStrategy(true);
        _enterStrategy->setProperty("ElaIconType", QChar((unsigned short)ElaIconType::CircleCheck));
        _ctrlAndEnterStrategy->setProperty("ElaIconType", QChar((unsigned short)ElaIconType::Circle));
    });

    connect(_ctrlAndEnterStrategy, &QAction::triggered, this, [=]() {
        _inputEditFrame->setSendMsgStrategy(false);
        _ctrlAndEnterStrategy->setProperty("ElaIconType", QChar((unsigned short)ElaIconType::CircleCheck));
        _enterStrategy->setProperty("ElaIconType", QChar((unsigned short)ElaIconType::Circle));
    });

    connect(_inputEditFrame, &SSTextEdit::sigSendMsgTrigger, this, [=]() {
        emit _sendButton->click();
    });
}
ConversationFriendPage::ConversationFriendPage(const UserBaseInfoData& userInfo,QWidget *parent)
    : QWidget(parent)
{
    _userInfo = userInfo;

    initWindow();

    initEdgeLayout();

    initContent();

    initConnectFunc();

}


ConversationFriendPage::~ConversationFriendPage(){}

void ConversationFriendPage::scrollMsgViewToBottom() {
    QTimer::singleShot(100,this,[=]() {
        _msgListView->scrollToBottom();
    });
}

void ConversationFriendPage::insertMsgBubble(const ChatMessage& msg) const {
    _msgBubbleModel->addMsg(msg);
    QTimer::singleShot(100,this,[=]() {
        _msgListView->scrollToBottom();
    });
}

void ConversationFriendPage::initWindow() {
    setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    // user name
    _userNameButton      =      new ElaToolButton(this);
    _videoButton         =      new ElaToolButton(this);
    _createGroupButton   =      new ElaToolButton(this);
    _moreOptionButton    =      new ElaToolButton(this);
    _msgListView         =      new MsgBubbleView(this);
    _msgBubbleDelegate   =      new MsgBubbleDelegate;
    _msgBubbleModel      =      new MsgBubbleModel;
    _toolLayout          =      new QHBoxLayout;
}

void ConversationFriendPage::initEdgeLayout() {

    _toolLayout->addWidget(_userNameButton);
    _toolLayout->addStretch();
    _toolLayout->addWidget(_videoButton);
    _toolLayout->addWidget(_createGroupButton);
    _toolLayout->addWidget(_moreOptionButton);
    _toolLayout->setContentsMargins(10,0,0,0);

    auto * _toolWid = new QWidget(this);
    _toolWid->setContentsMargins(0,10,0,20);
    _toolWid->setObjectName("_toolWid");
    _toolWid->setLayout(_toolLayout);
    _toolWid->setStyleSheet("#_toolWid {border: none; background-color: rgb(242, 242, 242);border-top-right-radius: 30px;}");

    _msgListView->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::MinimumExpanding);
    _msgListView->setMinimumWidth(500);
    _msgListView->setMaximumHeight(1000);
    _msgListView->setMinimumHeight(300);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(_toolWid);
    mainLayout->addWidget(_msgListView);

    this->setLayout(mainLayout);
    this->setContentsMargins(0,0,0,0);
}
void ConversationFriendPage::initContent() {
    _userNameButton->setText(_userInfo.username);
    _userNameButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    _videoButton->setElaIcon(ElaIconType::CircleVideo);
    _videoButton->setIconSize(QSize(32,32));
    _videoButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _createGroupButton->setElaIcon(ElaIconType::CommentPlus);
    _createGroupButton->setIconSize(QSize(32,32));
    _createGroupButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _moreOptionButton->setElaIcon(ElaIconType::CircleEllipsis);
    _moreOptionButton->setIconSize(QSize(32,32));
    _moreOptionButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    _msgListView->setObjectName("_msgListView");
    _msgListView->setLayoutMode(QListView::Batched);
    _msgListView->setResizeMode(QListView::Adjust);
    _msgListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _msgListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _msgListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _msgListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _msgListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _msgListView->setBatchSize(20);
    _msgListView->setSpacing(2);
    _msgListView->setWordWrap(true);
    _msgListView->setUniformItemSizes(false);
    _msgListView->setStyleSheet("#_msgListView { background-color: rgb(242, 242, 242); border : none;}");
    _msgListView->setModel(_msgBubbleModel);
    _msgListView->setItemDelegate(_msgBubbleDelegate);
    _msgListView->show();
}
void ConversationFriendPage::initConnectFunc() {
    // connect(_userNameButton,&QPushButton::clicked,[=]() {
    //     // TODO: remark and region need to get from server
    //     UserInfo userInfo{
    //         Friends,
    //         static_cast<int>(std::difftime(GetCurTime::getTimeObj()->getCurTimeStamp(),_userInfo.createTime)/ (60 * 60 * 24)) + 1,
    //         static_cast<int>(_userInfo.thumbUpCount), _userInfo.ssid, _userInfo.username, "", _userInfo.personalSign,
    //         _userInfo.avatarPath, {""}
    //     };
    //     UserPage * wid = g_pUserPage(Friends,userInfo,{});
    //     QPoint globalPos = QCursor::pos();
    //     wid->showAt(globalPos + QPoint(10,10));
    // });

    // connect(_videoButton,&ElaToolButton::clicked,[=]() {
    //     emit g_pCommonData->sigCallVideoToOtherUser(_userInfo.ssid);
    // });

    // connect(this, &ConversationFriendPage::sigCallVideoByOtherButton, [=]() {
    //     emit g_pCommonData->sigCallVideoToOtherUser(_userInfo.ssid);
    // });
}

ConversationGroupPage::ConversationGroupPage(
    const GroupBaseInfoData& groupBaseInfo,
    const QList<GroupMemberInfoData>& groupMemberInfo,
    QWidget *parent
    )
    : QWidget(parent)
{
    _groupBaseInfo = groupBaseInfo;
    _groupMemberInfo = groupMemberInfo;

    qint32 curUserSSID = CommonData::getInstance()->getCurUserInfo().ssid;
    _curType = Group_Member;
    // if (groupBaseInfo.admins.contains(curUserSSID)) {
//         _curType = Group_OP;
//     }
    if (groupBaseInfo.createSSID == curUserSSID) {
        _curType = Group_Creater;
    }

    initWindow();

    initEdgeLayout();

    initContent();

    initConnectFunc();
}

ConversationGroupPage::~ConversationGroupPage(){}

void ConversationGroupPage::scrollMsgViewToBottom() {
    QTimer::singleShot(300,this,[=]() {
        _msgListView->scrollToBottom();
    });
}

void ConversationGroupPage::insertMsgBubble(const ChatMessage& msg) const {
    _msgBubbleModel->addMsg(msg);
    QTimer::singleShot(300,this,[=]() {
        _msgListView->scrollToBottom();
    });
}

void ConversationGroupPage::initWindow() {
    setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    // group name
    // _callButton          =      new ElaToolButton(this);
    // _videoButton         =      new ElaToolButton(this);
    _groupNameButton     =      new ElaToolButton(this);
    _fileOfGroup         =      new ElaToolButton(this);
    _inviteAddButton     =      new ElaToolButton(this);
    _moreOptionButton    =      new ElaToolButton(this);
    _msgListView         =      new MsgBubbleView(this);
    _msgBubbleDelegate   =      new MsgBubbleDelegate;
    _msgBubbleModel      =      new MsgBubbleModel;
    _toolLayout          =      new QHBoxLayout;
}

void ConversationGroupPage::initEdgeLayout() {
    // toolbar layout
    _toolLayout->addWidget(_groupNameButton);
    _toolLayout->addStretch();
    // _toolLayout->addWidget(_callButton);
    // _toolLayout->addWidget(_videoButton);
    _toolLayout->addWidget(_fileOfGroup);
    _toolLayout->addWidget(_inviteAddButton);
    _toolLayout->addWidget(_moreOptionButton);
    _toolLayout->setContentsMargins(10,0,0,0);

    // toolbar wid to set style
    auto * _toolWid = new QWidget(this);
    _toolWid->setContentsMargins(0,10,0,20);
    _toolWid->setObjectName("_toolWid");
    _toolWid->setLayout(_toolLayout);
    _toolWid->setStyleSheet("#_toolWid {border: none; background-color: rgb(242, 242, 242);border-top-right-radius: 30px;}");

    // set msglist size
    _msgListView->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::MinimumExpanding);
    _msgListView->setMinimumWidth(500);
    _msgListView->setMaximumHeight(1000);
    _msgListView->setMinimumHeight(300);

    // main layout for toolbar and msglist
    auto *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(_toolWid);
    mainLayout->addWidget(_msgListView);

    this->setLayout(mainLayout);
    this->setContentsMargins(0,0,0,0);
}

void ConversationGroupPage::initContent() {
    // tool button settings
//只注释了下面两行
    _groupNameButton->setText(_groupBaseInfo.groupName);
    _groupNameButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    // _callButton->setElaIcon(ElaIconType::CirclePhone);
    // _callButton->setIconSize(QSize(32,32));
    // _callButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    // _videoButton->setElaIcon(ElaIconType::CircleVideo);
    // _videoButton->setIconSize(QSize(32,32));
    // _videoButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _fileOfGroup->setElaIcon(ElaIconType::Folder);
    _fileOfGroup->setIconSize(QSize(32,32));
    _fileOfGroup->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _inviteAddButton->setElaIcon(ElaIconType::CommentPlus);
    _inviteAddButton->setIconSize(QSize(32,32));
    _inviteAddButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    _moreOptionButton->setElaIcon(ElaIconType::CircleEllipsis);
    _moreOptionButton->setIconSize(QSize(32,32));
    _moreOptionButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // view setting
    _msgBubbleDelegate->setGroupMode(true);
    _msgListView->setObjectName("_msgListView");
    _msgListView->setLayoutMode(QListView::Batched);
    _msgListView->setResizeMode(QListView::Adjust);
    _msgListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _msgListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _msgListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _msgListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _msgListView->setBatchSize(20);
    _msgListView->setSpacing(2);
    _msgListView->setWordWrap(true);
    _msgListView->setUniformItemSizes(false);
    _msgListView->setStyleSheet("#_msgListView { background-color: rgb(242, 242, 242); border : none;}");
    _msgListView->setModel(_msgBubbleModel);
    _msgListView->setItemDelegate(_msgBubbleDelegate);
    _msgListView->show();
}

void ConversationGroupPage::initConnectFunc() {
    connect(_groupNameButton,&QPushButton::clicked,[=]() {
        // TODO: remark and region need to get from server
        // GroupInfo groupInfo{
        //     _curType,_groupBaseInfo.ssidGroup,_groupBaseInfo.groupName,"",
        //     _groupBaseInfo.profile,_groupBaseInfo.avatarPath,static_cast<int>(_groupMemberInfo.count()),
        //     {
        //         {"这是一个公告"}
        //     }
        // };
        // UserPage * wid = g_pUserPage(_curType,{},groupInfo);
        // QPoint globalPos = QCursor::pos();
        // wid->showAt(globalPos + QPoint(10,10));
    });
}

ConversationPage::ConversationPage(ConversationType type,const MsgCombineData& dto ,QWidget * parent)
    : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    auto * _layout = new QVBoxLayout;
    auto * _inputWid = new InputWidget(this);
    _layout->setContentsMargins(0,0,5,0);
    _layout->setSpacing(0);
    qint32 _curSSID = CommonData::getInstance()->getCurUserInfo().ssid;
    QString _curName = CommonData::getInstance()->getCurUserInfo().username;
    if(type == ConversationType::Friend){
        _cfP = new ConversationFriendPage(
            dto.userBaseInfo,this
            );
        _layout->addWidget(_cfP);
        _layout->addWidget(_inputWid);

        // handle file button click
        connect(_inputWid, &InputWidget::sigFileBtnClicked, this, [=](const QString& filePath) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to open file: " + filePath);
                return;
            }

            QByteArray fileData = file.readAll();
            file.close();

            // Detect file extension
            QString fileExt = detectFileExtension(fileData, filePath);
            if (fileExt.isEmpty()) {
                fileExt = ".bin";  // Default extension for unknown files
            }

            // Generate unique file ID with extension
            QString fileId = "file_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8) + fileExt;
            QByteArray base64Data = fileData.toBase64();

            // Create file message
            qint64 curTimeStamp = GetCurTime::getTimeObj()->getCurTimeStamp();
            MessageContentData msgDto{
                _curSSID,
                ContentType::File,  // Use File type for non-image files
                filePath.split('/').last(),  // Use filename as content
                {fileId},  // fileId field (now with extension)
                {
                    1,
                    dto.userBaseInfo.ssid
                },
                curTimeStamp
            };

            SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
                QString("Detected file type: %1 for file %2").arg(fileExt).arg(filePath));

            // Display in UI
            _cfP->insertMsgBubble({QString::number(_curSSID), _curName,
                                   "[文件: " + filePath.split('/').last() + "]", CommonData::getInstance()->getCurUserInfo().avatarPath, true});

            // Update card
            MessagePage * msgPage = dynamic_cast<MessagePage*>(parent->parent());
            msgPage->_ssidLinkCardHash[dto.userBaseInfo.ssid]->setTimeContent(
                QString::fromStdString(GetCurTime::getTimeObj()->getMsgTypeTime(curTimeStamp)),
                Qt::gray, QFont());
            msgPage->_ssidLinkCardHash[dto.userBaseInfo.ssid]->setSubTitle("[文件: " + filePath.split('/').last() + "]");

            // Save file to project file_storage directory
            QString storageDir = QCoreApplication::applicationDirPath() + "/file_storage";
            QDir dir;
            if (!dir.exists(storageDir)) {
                dir.mkpath(storageDir);
            }
            QString fullFilePath = storageDir + "/" + fileId;
            QFile outFile(fullFilePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                outFile.write(fileData);
                outFile.close();
            }

            CommonData::getInstance()->setMessageContentData({msgDto});
        });

        // send msg by myself
        connect(_inputWid, &InputWidget::sigSendBtnClicked, this, [=](const QString& html) {
            QString html_cp = html;
            // add msg pic to the tmp
            QMap<QString,QImage>& cacheImage = _inputWid->_inputEditFrame->getImageTmpMap();
            QList<QString> fileIDs;
            if (!cacheImage.isEmpty()) {
                for (auto imageIt = cacheImage.begin(); imageIt != cacheImage.end(); imageIt++) {
                    QString imageName = imageIt.key();
                    // 保存图片到临时目录
                    CommonData::getInstance()->saveImageToTemp(imageName, imageIt.value());
                    QString localPath = CommonData::getInstance()->getImageTempFilePath(imageName);
                    // 替换html中的图片引用为本地路径
                    html_cp.replace(imageName, localPath);
                    fileIDs.append(imageName);
                }
                cacheImage.clear();
            }
            _inputWid->_inputEditFrame->clear();

            _cfP->insertMsgBubble({QString::number(_curSSID),_curName,
                                   html_cp,CommonData::getInstance()->getCurUserInfo().avatarPath,true});

            // store msg
            qint64 curTimeStamp = GetCurTime::getTimeObj()->getCurTimeStamp();
            ContentType msgType = fileIDs.isEmpty() ? ContentType::Text : ContentType::Image;
            MessageContentData msgDto{
                _curSSID,
                msgType,
                html_cp,
                fileIDs,  // fileId字段（单数）
                {
                    1,
                    dto.userBaseInfo.ssid
                },
                curTimeStamp
            };

            // get grandfather to link card and set content display
            MessagePage * msgPage = dynamic_cast<MessagePage*>(parent->parent());

            // replace image url to [图片] placeholders for card subtitle
            QString cardText = html_cp;
            QRegularExpression imgRegex("<img[^>]*>", QRegularExpression::CaseInsensitiveOption);
            cardText.replace(imgRegex,"[图片]");
            QFont font;
            QTextDocument docu;
            docu.setHtml(cardText);
            font.setPointSize(9);

            msgPage->_ssidLinkCardHash[dto.userBaseInfo.ssid]->setTimeContent(QString::fromStdString(
                                                                                  GetCurTime::getTimeObj()->getMsgTypeTime(curTimeStamp)),
                                                                              Qt::gray,font);
            msgPage->_ssidLinkCardHash[dto.userBaseInfo.ssid]->setSubTitle(docu.toPlainText());

            // sync with server
            CommonData::getInstance()->setMessageContentData({msgDto});
        });
    }
    else if(type == ConversationType::Group){
        _cgP = new ConversationGroupPage(
            dto.groupBaseInfo,
            dto.groupMemberInfo,
            this
            );
        auto * listAndInputLayout = new QHBoxLayout;
        listAndInputLayout->setContentsMargins(0,0,0,0);
        listAndInputLayout->setSpacing(0);

        // group member list dock
        _memberOfGroupList = new GroupMemberDock(this);
        _memberOfGroupList->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
        _memberOfGroupList->setFixedWidth(120);
        _memberOfGroupList->setMaximumHeight(_inputWid->height());

        std::vector<qint32>uids;
        for(const auto& it:dto.groupMemberInfo)
        {
            uids.push_back(it.ssidMember);
        }
        ClientRequestHandler::getInstance()->queryUserInfoBatch(uids,[this](const QJsonObject& obj){
            QHash<qint32,UserBaseInfoData> tmpUserHash;
            if(obj.contains("users")&&obj["users"].isArray())
            {
                QJsonArray users=obj["users"].toArray();
                for(const QJsonValue val:users)
                {
                    UserBaseInfoData user;
                    user.ssid=val["uid"].toInt();
                    user.username=val["username"].toString();
                    if(user.avatarPath=="-1"||user.avatarPath.isEmpty())
                    {
                        user.avatarPath=":/include/Image/Cirno.jpg";
                    }
                    _memberOfGroupList->addMember(user.avatarPath,QString::number(user.ssid),user.username);
                    tmpUserHash.insert(user.ssid,user);
                }
                //         // connect group member single clicked
                //lambda会保存tmpUserHash的副本,即使它被销毁
                //         connect(_memberOfGroupList,&GroupMemberDock::sigClickedMember,this,[=](QString ssid) {
                //             // TODO : send request to server
                //             UserBaseInfoDTO user = tmpUserHash.value(ssid);
                //             UserType clickedType = Strangers;
                //             if ( g_pCommonData->getCurUserInfo().ssid == ssid) {
                //                 clickedType = Myself;
                //             }
                //             if (g_pCommonData->isCurUserFriend(ssid)) {
                //                 clickedType = Friends;
                //             }

                //             UserInfo uInfo{
                //                 clickedType,
                //                 static_cast<int>(std::difftime(GetCurTime::getTimeObj()->getCurTimeStamp(),user.createTime) / (60 * 60 * 24) + 1),
                //                 static_cast<int>(user.thumbUpCount) , user.ssid, user.username, "", user.personalSign,
                //                 user.avatarPath,{""}
                //             };

                //             UserPage * wid = g_pUserPage(clickedType,uInfo,{});

                //             QPoint globalPos = QCursor::pos();
                //             QPoint offset(wid->width(),wid->height());
                //             wid->showAt(globalPos-offset);
                //         });
            }
        });
        _memberOfGroupList->setObjectName(QString::fromUtf8("_memberOfGroupList"));
        _memberOfGroupList->setStyleSheet("#_memberOfGroupList {background-color: rgb(242, 242, 242);}");

        listAndInputLayout->addWidget(_inputWid);
        listAndInputLayout->addWidget(_memberOfGroupList);

        _layout->addWidget(_cgP);
        _layout->addItem(listAndInputLayout);

        // handle file button click for group
        connect(_inputWid, &InputWidget::sigFileBtnClicked, this, [=](const QString& filePath) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Failed to open file: " + filePath);
                return;
            }

            QByteArray fileData = file.readAll();
            file.close();

            // Detect file extension
            QString fileExt = detectFileExtension(fileData, filePath);
            if (fileExt.isEmpty()) {
                fileExt = ".bin";  // Default extension for unknown files
            }

            // Generate unique file ID with extension
            QString fileId = "file_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8) + fileExt;

            // Create file message
            qint64 curTimeStamp = GetCurTime::getTimeObj()->getCurTimeStamp();
            MessageContentData msgDto{
                _curSSID,
                ContentType::File,  // Use File type for non-image files
                filePath.split('/').last(),  // Use filename as content
                {fileId},  // fileId field (now with extension)
                {
                    2,
                    dto.groupBaseInfo.ssidGroup
                },
                curTimeStamp
            };

            SSLog::log(SSLog::LogLevel::SS_INFO, QString(__FILE__), __LINE__,
                QString("Detected file type: %1 for file %2").arg(fileExt).arg(filePath));

            // Display in UI
            _cgP->insertMsgBubble({QString::number(_curSSID), _curName,
                                   "[文件: " + filePath.split('/').last() + "]", CommonData::getInstance()->getCurUserInfo().avatarPath, true});

            // Update card
            MessagePage * msgPage = dynamic_cast<MessagePage*>(parent->parent());
            msgPage->_ssidLinkCardHash[dto.groupBaseInfo.ssidGroup]->setTimeContent(
                QString::fromStdString(GetCurTime::getTimeObj()->getMsgTypeTime(curTimeStamp)),
                Qt::gray, QFont());
            msgPage->_ssidLinkCardHash[dto.groupBaseInfo.ssidGroup]->setSubTitle("[文件: " + filePath.split('/').last() + "]");

            // Save file to project file_storage directory
            QString storageDir = QCoreApplication::applicationDirPath() + "/file_storage";
            QDir dir;
            if (!dir.exists(storageDir)) {
                dir.mkpath(storageDir);
            }
            QString fullFilePath = storageDir + "/" + fileId;
            QFile outFile(fullFilePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                outFile.write(fileData);
                outFile.close();
            }

            // Send to server
            CommonData::getInstance()->setMessageContentData({msgDto});
        });

        connect(_inputWid, &InputWidget::sigSendBtnClicked, this, [=](const QString& html) {
            QString html_cp = html;
            // add msg pic to the tmp
            QMap<QString,QImage>& cacheImage = _inputWid->_inputEditFrame->getImageTmpMap();
            QList<QString> fileIds;
            if (!cacheImage.isEmpty()) {
                for (auto imageIt = cacheImage.begin(); imageIt != cacheImage.end(); imageIt++) {
                    QString imageName = imageIt.key();
                    // 保存图片到临时目录
                    CommonData::getInstance()->saveImageToTemp(imageName, imageIt.value());
                    QString localPath = CommonData::getInstance()->getImageTempFilePath(imageName);
                    // 替换html中的图片引用为本地路径
                    html_cp.replace(imageName, localPath);
                    fileIds.append(imageName);
                }
                cacheImage.clear();
            }
            _inputWid->_inputEditFrame->clear();
            _cgP->insertMsgBubble({QString::number(_curSSID),_curName,
                                   html_cp,CommonData::getInstance()->getCurUserInfo().avatarPath,true});

            // store msg
            qint64 curTimeStamp = GetCurTime::getTimeObj()->getCurTimeStamp();
            ContentType msgType = fileIds.isEmpty() ? ContentType::Text : ContentType::Image;
            MessageContentData msgDto{
                _curSSID,
                msgType,
                html_cp,
                fileIds,  // fileId字段（单数）
                {
                    2,
                    dto.groupBaseInfo.ssidGroup
                },
                curTimeStamp
            };

            // get grandfather to link card and set content display
            MessagePage * msgPage = dynamic_cast<MessagePage*>(parent->parent());

            // replace image url to [图片] placeholders for card subtitle
            QString cardText = html_cp;
            QRegularExpression imgRegex("<img[^>]*>", QRegularExpression::CaseInsensitiveOption);
            cardText.replace(imgRegex,"[图片]");
            QFont font;
            QTextDocument docu;
            docu.setHtml(cardText);
            font.setPointSize(9);
            msgPage->_ssidLinkCardHash[dto.groupBaseInfo.ssidGroup]->setTimeContent(QString::fromStdString(
                                                                                        GetCurTime::getTimeObj()->getMsgTypeTime(curTimeStamp)),
                                                                                    Qt::gray,font);
            msgPage->_ssidLinkCardHash[dto.groupBaseInfo.ssidGroup]->setSubTitle(docu.toPlainText());

            // sync with server
            CommonData::getInstance()->setMessageContentData({msgDto});
        });
    }
    this->setLayout(_layout);
    this->setObjectName("ConversationPage");
    this->setStyleSheet("#ConversationPage{background-color: rgb(242, 242, 242);border-bottom-left-radius: 30px;border-top-right-radius: 30px;}");
}

ConversationPage::~ConversationPage()
{

}

void ConversationPage::scrollMsgViewToBottom() {
    if (_cfP == nullptr)
        _cgP->scrollMsgViewToBottom();
    else
        _cfP->scrollMsgViewToBottom();
}
