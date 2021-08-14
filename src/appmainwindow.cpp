#include <qxstl/serialization.hpp>
#include <qxstl/FormLoader.hpp>

#include "appmainwindow.hpp"

namespace qx = qxstl::event;

AppMainWindow::AppMainWindow()
    : loader{FormLoader(this, ":/assets/user_interface.ui")}
{

    form = loader.GetForm();

    // Load widget from XML gui layout file by unique identifier name. 
    this->entry_disk_path = loader.find_child<QLineEdit>("entry_disk_path");
    this->entry_disk_path->setText("Loaded Ok.");

    this->spin_memory = loader.find_child<QSpinBox>("spin_memory");

    this->btn_run = loader.find_child<QPushButton>("btn_run");

    this->proc = new QProcess(this);

    const auto program = QString{"qemu-system-x86_64"};

    loader.on_button_clicked("btn_run", [=]
    {
        QString path = this->entry_disk_path->text();

        QString memory = QString::number( this->spin_memory->value() );

        bool audio_enabled = loader.is_checkbox_checked("enable_audio");

        auto list = QStringList{"-enable-kvm"
                            , "-m",      memory      // RAM Memory assigned to VM  
                            , "-smp",    "2"         // Number of cores
                            , "-net",    "nic"
                            , "-net",    "user"
                            , "-usb"      
                            , "-device", "usb-tablet"
                            , "-boot",   "d"          // CDROM boot 
                            , "-cdrom",   path
                            };

        if(audio_enabled){
            std::cout << " [TRACE] Audo enabled Ok. " << std::endl;
            list.push_back("-audiodev"); list.push_back("pa,id=snd0");
            list.push_back("-device");  list.push_back("ich9-intel-hda");
            list.push_back("-device");  list.push_back("hda-output,audiodev=snd0");
        }
        
        // QMessageBox::about(nullptr, "Title", "Button clicked.");
        proc->start(program, list);
    });

    //========= Create Tray Icon =======================//

   // // Do not quit when user clicks at close button
   // this->setAttribute(Qt::WA_QuitOnClose, false);

   // tray_icon = qx::make_window_toggle_trayicon(
   //     this,
   //     ":/assets/appicon.png"
   //     , "Tray Icon Test"
   //     );

    //========= Load Application state =================//

    this->setWindowAlwaysOnTop();

    // ========== Event Handlers of tray Icon ===============================//

    // Toggle this main window visible/hidden when user clicks at Tray Icon.
    // QObject::connect(tray_icon, &QSystemTrayIcon::activated
    //                  , [&self = *this](QSystemTrayIcon::ActivationReason r)
    //                  {
    //                      // User clicked at QSystemTrayIcon
    //                      if(r  == QSystemTrayIcon::Trigger)
    //                      {
    //                          if(!self.isVisible())
    //                              self.show();
    //                          else
    //                              self.hide();
    //                          return;
    //                      }
    //                      // std::cout << " [TRACE] TrayIcon Clicked OK." << std::endl;
    //                  });

    // ========== Set Event Handlers of Application Launcher Tab ============//

    // Enable Drag and Drop Event
    this->setAcceptDrops(true);

    // Register pointer to static member function
    loader.on_button_clicked("btn_quit_app",
                             [self = this]
                             {
                                 // self->save_settings();
                                 QApplication::quit();
                             });

    loader.on_button_clicked("btn_show_help", &QWhatsThis::enterWhatsThisMode);


} // --- End of CustomerForm ctor ------//

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

/// Make this window stay alwys on top
void
AppMainWindow::setWindowAlwaysOnTop()
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}


void
AppMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
#if 1

    const QMimeData *mimeData = event->mimeData();
    std::cout << "Drag Event" << std::endl;
    
    if (!mimeData->hasUrls()){ return; }
    auto url = mimeData->urls()[0];
    QString path;

    if (url.isLocalFile())
        path = mimeData->urls()[0].toLocalFile();
    else
        path = mimeData->urls()[0].toString();

    std::cout << " [TRACE] Dragged file: " << path.toStdString() << "\n";
    // this->tview_disp->addItem(path); 

    this->entry_disk_path->setText(path);

#endif

}
