/**  Brief: Application Main Window
 *   Author: Caio Rodrigues - caiorss [at] rodrigues [at] gmail [dot] com
 *
 *
 ************************************************************/
#include <iostream>

#include <QApplication>
#include "appmainwindow.hpp"

constexpr const char* APP_NAME = "qqemu";

int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);   

    AppMainWindow maingui;
    maingui.setWindowIcon(QIcon(":/assets/appicon.png"));
    maingui.showNormal();


    return app.exec();
}

