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

    QSharedMemory shmem;
    shmem.setKey("qemu-launcher-key");

    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);   

    if( !shmem.create(1) )
    {
        QMessageBox::warning( nullptr
                             ,"Error report"
                             ,"Only one application instance is allowed.");

        std::cerr << " [ERROR] Only one application instance is allowed." << std::endl;
        return -1;
    }



    AppMainWindow maingui;
    maingui.setWindowIcon(QIcon(":/assets/appicon.png"));
    maingui.showNormal();


    return app.exec();
}

