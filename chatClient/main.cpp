#include "mainwindow.h"
#include"page-view/landpage.h"
#include <QApplication>
#include<ElaApplication.h>
#include<utils/net-work/clientconserver.h>
#include"page-view/archpage.h"
#include<utils/log/logfile.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    
    // 初始化日志系统 - 必须在其他组件之前
    SSLog::initLogFile("log");
    
    LandPage::getInstance()->show();
    ClientConServer::getInstance()->connectToServer();
    // MainWindow w;
    // w.show();
    // ArchPage::getInstance()->show();
    return a.exec();
}
