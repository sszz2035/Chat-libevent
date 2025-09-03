#include "mainwindow.h"
#include"page-view/landpage.h"
#include <QApplication>
#include<ElaApplication.h>
#include<utils/net-work/clientconserver.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    LandPage::getInstance()->show();
    MainWindow w;
    w.show();
    ClientConServer s;
    s.connectToServer();
    return a.exec();
}
