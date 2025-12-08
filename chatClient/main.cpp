#include "mainwindow.h"
#include"page-view/landpage.h"
#include <QApplication>
#include<ElaApplication.h>
#include<utils/net-work/clientconserver.h>
#include"page-view/archpage.h"
#include<utils/log/logfile.h>
#include "core/database/databasemanager.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    
    // 初始化日志系统 - 必须在其他组件之前
    SSLog::initLogFile("log");

    // 初始化数据库
    if (!DatabaseManager::instance().initDatabase("chatclient.db")) {
        SSLog::log(SSLog::LogLevel::SS_ERROR, QString(__FILE__), __LINE__, "Database initialization failed!");
    }

    LandPage::getInstance()->show();
    ClientConServer::getInstance()->connectToServer();
    // MainWindow w;
    // w.show();
    // ArchPage::getInstance()->show();
    return a.exec();
}
