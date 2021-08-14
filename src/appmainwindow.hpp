#ifndef APPMAINWINDOW_HPP
#define APPMAINWINDOW_HPP

#include <QtWidgets>
#include <QProcess> 

//------ Headers of helper classes and namespaces -------//
#include <qxstl/event.hpp>
#include <qxstl/FormLoader.hpp>
#include <qxstl/RecordTableModel.hpp>

// ----- Headers of Domain Classes -----//
#include "filebookmarkitemmodel.hpp"
#include "FileBookmarkItem.hpp"
#include "tab_applicationlauncher.hpp"
#include "tab_desktopbookmarks.hpp"



class AppMainWindow: public QMainWindow
{
private:
    FormLoader   loader;
    QWidget*     form;

    //======== TrayIcon =============================//
    QSystemTrayIcon* tray_icon;
    QLineEdit*       entry_disk_path;
    QPushButton*     btn_run;
    QSpinBox*        spin_memory;
    QCheckBox*       enable_audio;
    
    QTimer*          timer;

    QProcess* proc;
public:


    AppMainWindow();
    /// Make this window stay alwys on top
    void setWindowAlwaysOnTop();

    void dragEnterEvent(QDragEnterEvent* event) override;

#if 0
    // See: https://stackoverflow.com/questions/18934964
    bool eventFilter(QObject* object, QEvent* event) override
    {

        if(object == this->app_registry && event->type() == QEvent::KeyRelease)
        {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if(ke->key() == Qt::Key_Enter || Qt::Key_Return)
            {
                std::cout << " [INFO] User pressed Return on QListWidget " << std::endl;
            }
            return false;
        }
        return false;
    }
#endif

};



#endif // APPMAINWINDOW_HPP
