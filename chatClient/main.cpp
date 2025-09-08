#include "mainwindow.h"
#include"page-view/landpage.h"
#include <QApplication>
#include<ElaApplication.h>
#include<utils/net-work/clientconserver.h>
#include"page-view/archpage.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    LandPage::getInstance()->show();
    ClientConServer::getInstance()->connectToServer();
    // MainWindow w;
    // w.show();
    // ArchPage::getInstance()->show();
    return a.exec();
}
