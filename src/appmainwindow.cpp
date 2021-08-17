#include <qxstl/serialization.hpp>
#include <qxstl/FormLoader.hpp>

#include <QUuid>

#include "appmainwindow.hpp"

namespace qx = qxstl::event;

AppMainWindow::AppMainWindow()
    : loader{FormLoader(this, ":/assets/user_interface.ui")}
{
    form = loader.GetForm();
    loader.combobox_add_item(COMBOBOX_QEMU, "qemu-system-x86_64");
    loader.combobox_add_item(COMBOBOX_QEMU, "qemu-system-i386");
    // Load widget from XML gui layout file by unique identifier name. 
    this->entry_disk_path = loader.find_widget<QLineEdit>(ENTRY_DISK_PATH);
    this->spin_memory = loader.find_widget<QSpinBox>(SPINBOX_MEMORY);
    this->btn_run = loader.find_widget<QPushButton>(BTN_RUN);
    this->proc = new QProcess(this);


    QObject::connect(proc, &QProcess::stateChanged, this, &AppMainWindow::qemu_state_changed);
    loader.on_button_clicked(BTN_STOP, this, &AppMainWindow::kill_qemu_process);
    loader.on_button_clicked(BTN_INSTALL_ICON, this, &AppMainWindow::install_desktop_icon);
    loader.on_button_clicked(BTN_RUN, this, &AppMainWindow::run_qemu_process);

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

void AppMainWindow::kill_qemu_process() 
{
    this->proc->kill();
}

void AppMainWindow::run_qemu_process()
{
    //QString{"qemu-system-x86_64"};
    const auto program = loader.combobox_selected_text(COMBOBOX_QEMU);
    std::cout << " [TRACE] Program = " << program.toStdString() << '\n';

    QString path = this->entry_disk_path->text();
    QString vmname = QFileInfo(path).fileName().section(".", 0, 0);

    QString machine_uuid = QUuid::createUuid().toString();
    machine_uuid = machine_uuid.replace("{", "").replace("}", "");

    std::cout << " [TRACE] UUID = " << machine_uuid.toStdString() << "\n";

    display_text = QString(R"( 
  [-------------------------------------------------------------]

    Connect to QEMU console for the virtual machine using the command: 

        $  socat STDIO unix-connect:/tmp/qemu-monitor-socket.sock
    or:
        $  rlwrap socat STDIO unix-connect:/tmp/qemu-monitor-socket.sock

  [-------------------------------------------------------------]

       Machine Name = %1
       Machine UUID = %2
    )")
                       .arg(vmname, machine_uuid);

    auto list = QStringList{
        "-enable-kvm" // Enable KVM (Kernel Virtual Machine) accelerator
        ,"-smp", "2" // Number of cores
        , "-net", "nic" // Add network interface card (NIC)
        , "-usb", "-device", "usb-tablet", "-boot", "d" // CDROM boot

        // Daemonize => Uncomment the next line for daemonizing QEMU
        // , "-daemonize"

        // Define process name for this virtual machine
        // See: https://blog.stefan-koch.name/2020/05/24/managing-qemu-vms
        //
        // -name <VIRTUAL-MACHINE-NAME>,process=<PROCESS-NAME>
        , "-name", QString("qemu-vm-%1,process=qemu-proc-%1").arg(vmname, vmname)

        // Universal unique identifier for virtual machine
        , "-uuid", machine_uuid //"909fa9bb-a57f-4f9a-a323-e4a74595b2f9"

        // QEMU QMP (Protocol)
        //  => https://wiki.qemu.org/QMP
        //  => https://git.qemu.org/?p=qemu.git;a=blob_plain;f=docs/interop/qmp-intro.txt;hb=HEAD
        // ----------------------------------
        , "-chardev", "socket,id=mon1,host=localhost,port=4444,server=on,wait=off", "-mon", "chardev=mon1,mode=control,pretty=on"

        // Run QEMU monitor console using unix domain socket 'file'
        // --------------------------------------------
        , "-monitor", "unix:/tmp/qemu-monitor-socket.sock,server,nowait"};

    if (path.endsWith(".iso"))
    {
        // Path to ISO disk cd/dvd image
        list.push_back("-cdrom");
        list.push_back(path);
    }

    if (path.endsWith("qcow"))
    {
        // -device ide-driver,..... ...
        list.push_back("-device");
        list.push_back("ide-drive,bus=ide.0,drive=MacHDD");

        // -drive id=MacHDD,format=qcow2,if=none,file=disk-image.qcow
        list.push_back("-drive");
        list.push_back(QString("id=MacHDD,format=qcow2,if=none,file=") + path);
    }

    if (loader.checkbox_is_checked(CHECKBOX_WINDOWS))
    {
        // -smbios-type 2
        list.push_back("-smbios");
        list.push_back("type=2");

        // -device ahci,id=ide
        list.push_back("-device");
        list.push_back("ahci,id=ide");

        // -cpu Penryn,kvm=off,vendor=GenuineIntel
        list.push_back("-cpu");
        list.push_back("Penryn,kvm=off,vendor=GenuineIntel");

        if (this->spin_memory->value() < 4096)
        {
            this->spin_memory->setValue(4096);
        }
    }

    // -m <RAM-MEMORY-AMOUNT>
    // RAM Memory assigned to VM
    QString memory = QString::number(this->spin_memory->value());
    list.push_back("-m");
    list.push_back(memory);

    if (loader.checkbox_is_checked("enable_audio"))
    {

        std::cout << " [TRACE] Audo enabled Ok. " << std::endl;
        list.push_back("-audiodev");
        list.push_back("pa,id=snd0");
        list.push_back("-device");
        list.push_back("ich9-intel-hda");
        list.push_back("-device");
        list.push_back("hda-output,audiodev=snd0");
    }

    if (loader.checkbox_is_checked(CHECKBOX_ETHERNET))
    {
        list.push_back("-net");
        list.push_back("user,id=internent");
    }

    // QMessageBox::about(nullptr, "Title", "Button clicked.");
    proc->start(program, list);
}

void AppMainWindow::install_desktop_icon() 
{
        QString desktop_path =
            QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0];

        qx::create_linux_desktop_shortcut(
              desktop_path
            , ":/assets/appicon.png"
            , "QT front-end for QEMU virtual machine emulator.");
}

void AppMainWindow::qemu_state_changed()
{
    bool flag = proc->state() == QProcess::Running;
    loader.widget_set_disabled(BTN_RUN, flag);
    loader.widget_set_disabled(BTN_STOP, !flag);
    loader.widget_set_disabled(ENTRY_DISK_PATH, flag);
    loader.widget_set_disabled(CHECKBOX_ETHERNET, flag);
    loader.widget_set_disabled(CHECKBOX_AUDIO, flag);
    loader.widget_set_disabled(SPINBOX_MEMORY, flag);
    loader.widget_set_disabled(COMBOBOX_QEMU, flag);

    if (proc->state() == QProcess::NotRunning)
    {
        std::fprintf(stdout, " [INFO] Process stopped. Ok. \n");
        loader.widget_setText(LABEL_STATUS_BAR, "Virtual machine stopped.");

        QString qemu_stdout = this->proc->readAllStandardOutput();
        QString qemu_stderr = this->proc->readAllStandardError();
        QString output;

        if (this->proc->exitCode() != 0)
        {
            output = " [QEMU ERROR] \n";
        }

        output = output + qemu_stdout + "\n\n" + qemu_stderr;
        loader.widget_setText(TEXTEDIT_DISPLAY, output);
    }

    if (proc->state() == QProcess::Running)
    {
        loader.widget_setText(LABEL_STATUS_BAR, "Virtual machine running.");

        loader.widget_setText(TEXTEDIT_DISPLAY, display_text);
    }
}
