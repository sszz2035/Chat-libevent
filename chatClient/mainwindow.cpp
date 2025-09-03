#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : ElaWindow(parent)
{
    //初始化窗口
    initWindow();
}

MainWindow::~MainWindow()
{

}


void MainWindow::initWindow()
{
    setFocusPolicy(Qt::StrongFocus);
    //调整窗口大小
    resize(1000, 650);

}

