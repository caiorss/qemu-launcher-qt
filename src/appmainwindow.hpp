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


constexpr const char* BTN_QUIT_APP        = "btn_quit_app";
constexpr const char* BTN_RUN             = "btn_run";
constexpr const char* BTN_STOP            = "btn_stop";
constexpr const char* BTN_INSTALL_ICON    = "btn_icon";
constexpr const char* BTN_FILE_ISO        = "btn_file_iso";
constexpr const char* BTN_FILE_QCOW       = "btn_file_qcow";
constexpr const char* BTN_SHOW_HELP       = "btn_show_help";
constexpr const char* BTN_CLEAR           = "btn_clear";
constexpr const char* BTN_REMOTE_SPICE    = "btn_remote_viewer";


constexpr const char* LABEL_STATUS_BAR    = "label_status_bar";
constexpr const char* CHECKBOX_ETHERNET   = "enable_ethernet";
constexpr const char* CHECKBOX_AUDIO      = "enable_audio";
constexpr const char* CHECKBOX_WINDOWS    = "enable_windows";
constexpr const char* CHECKBOX_SPICE      = "enable_spice";

constexpr const char* ENTRY_PATH_ISO     = "entry_disk_path_iso";
constexpr const char* ENTRY_PATH_QCOW     = "entry_disk_path_qcow";

constexpr const char* SPINBOX_MEMORY      = "spin_memory";
constexpr const char* TEXTEDIT_DISPLAY    = "display";
constexpr const char* COMBOBOX_QEMU       = "combobox_qemu";



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
    QProcess* proc_spice;
    QString display_text = "";
public:


    AppMainWindow();
    /// Make this window stay alwys on top
    void setWindowAlwaysOnTop();

   // void dragEnterEvent(QDragEnterEvent* event) override;

    void qemu_kill_process();

    void qemu_run_process();

    void install_desktop_icon();

    void qemu_state_changed();

    /// Run remote viewer process (spice protocol)
    void remote_viewer_run();

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
