#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<ElaWindow.h>
QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class MainWindow : public ElaWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initWindow();

};

#endif // MAINWINDOW_H
