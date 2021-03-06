#ifndef FORMLOADER_HPP
#define FORMLOADER_HPP

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>


// #include <iostream>
#include <cassert>
#include <string>

namespace qxstl::gui {

/**
  * Class FormLoader is a helper for loading QtWidgets dynamically
  * from a Form without compilation. */
class FormLoader
{
private:
    QString  formFile;
    QWidget* form;
    QWidget* m_parent;
public:

    FormLoader(QMainWindow* parent, QString path)
    {
        m_parent = parent;

        this->LoadForm(path);
        parent->setCentralWidget(form);
        parent->setWindowTitle(form->windowTitle());

        // Set Width and height
        parent->resize(form->width(), form->height());

        // Center Window in the screen
        parent->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                parent->size(),
                qApp->desktop()->availableGeometry()
                )
            );
    }

    virtual ~FormLoader() = default;

    // Forbid copy, aka Deep Copy
    FormLoader(FormLoader const&) = delete;
    FormLoader& operator=(FormLoader const&) = delete;

    void LoadForm(QString filePath)
    {
        QUiLoader loader;
        formFile = filePath;
        QFile file(filePath);
        // file.open(QFile::ReadOnly);
        form = loader.load(&file, nullptr);
        assert(form != nullptr);
        file.close();
    }

    QWidget* GetForm() { return form;  }

    /// @brief Find child widget in the user interface form file.
    template<typename T>
    T* find_widget(QString widget_name)
    {
        T* widget = form->findChild<T*>(widget_name);
        // Throws exception if widget is not found in order to
        // make the failure easier to trace.
        this->ensure_widget_loaded(widget, widget_name);
        return widget;
    }

    /// Types that models Sender type concept: QPushButton, QCheckBox
    template<typename Sender, typename Callback>
    void on_clicked(QString widget_name, Callback&& event_handler)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        this->ensure_widget_loaded(pSender, widget_name);
        QObject::connect(pSender, &Sender::clicked, event_handler);
    }

    template<typename Sender, typename Receiver, typename Method>
    void on_clicked(QString widget_name, Receiver pReceiver, Method&& receiver_method)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        this->ensure_widget_loaded(pSender, widget_name);
        QObject::connect(pSender, &Sender::clicked, [=]
                         {
                             // Member function pointer invocation
                             (pReceiver->*receiver_method)();
                         });
    }

    template<typename Sender, typename Callback>
    void on_src_clicked(QString widget_name, Callback&& event_handler)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        this->ensure_widget_loaded(pSender, widget_name);
        QObject::connect(pSender, &Sender::clicked, [=]{ event_handler(pSender); });
    }


    template<typename Callback>
    void on_button_clicked(QString widget_name, Callback&& event_handler)
    {
        this->on_clicked<QPushButton>(widget_name, event_handler);
    }

    template<typename Receiver, typename Method>
    void on_button_clicked(QString widget_name, Receiver pReceiver, Method&& receiver_method)
    {
        this->on_clicked<QPushButton>(widget_name, pReceiver, receiver_method);
    }

    /// Types that models Sender type concept: QListWidget, QTableWidget
    template<typename Sender, typename Callback>
    void on_double_clicked(QString widget_name, Callback&& event_handler)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        this->ensure_widget_loaded(pSender, widget_name);
        QObject::connect(pSender, &Sender::doubleClicked, event_handler);
    }


    template<typename Sender, typename Receiver, typename Method>
    void on_double_clicked(QString widget_name, Receiver pReceiver, Method&& receiver_method)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        QObject::connect(pSender, &Sender::doubleClicked, pReceiver, receiver_method);
    }

    /** Returns true if a given checkbox is checked */
    bool checkbox_is_checked(QString widget_name)
    {
        auto checkbox = this->find_widget<QCheckBox>(widget_name);
        return checkbox->checkState() == Qt::Checked;
        // return checkbox->isChecked();
    }

    /** If the flag is true, disables widget. 
     * It is a wrapper to QWidget::setDisabled(flag)
     * 
     ****************************************************/
    void widget_set_disabled(QString widget_name, bool flag)
    {
        auto widget = this->find_widget<QWidget>(widget_name);
        widget->setDisabled(flag);
    }

    void widget_setText(QString name, QString text)
    {        
        QWidget* self = this->find_widget<QWidget>(name);
        if( set_text<QLabel*>(self, text))           return;
        if( set_text<QLineEdit*>(self, text))        return;
        if( set_text<QTextEdit*>(self, text))        return; 
        if( set_text<QMessageBox*>(self, text))      return; 
        if( set_text<QAbstractButton*>(self, text) ) return;    
        // logger().log() << " [TRACE] Called function " << __FUNCTION__ << std::endl;
        self->setWindowTitle(text);  
    }

    QString lineEdit_text(QString name)
    {
        auto self = this->find_widget<QLineEdit>(name);
        return self->text();
    }

    /// @brief Add items to a combobox.
    void combobox_add_item(QString name, QString item)
    {
        auto self = this->find_widget<QComboBox>(name);
        self->addItem(item);
    }

    /// @brief Get selected item as string
    QString combobox_selected_text(QString name)
    {
        auto self = this->find_widget<QComboBox>(name);
        return self->currentText();
    }

    /// @brief Install event handler, that show a not-implemented message box, in a button. 
    void button_no_implemented(QString name)
    {
        this->on_button_clicked(name, [&]{
            QMessageBox msbgox;
            msbgox.setText("Error: this functionality was not implemented yet.");
            msbgox.exec();
        });
    }

private:

    /** Ensure that widget was loaded from XML. Throws exception if the widget
     *  cannot be found in the form file.
     * This function makes the error diagnosing easier.
     */
    void ensure_widget_loaded(QWidget* widget, QString widget_name)
    {
        if(widget != nullptr) { return; }
        using namespace std::string_literals;
        throw std::runtime_error("Error: Unable to load widget named: <"s
                                 + widget_name.toStdString()
                                 + "> from the form file "s + formFile.toStdString());
    }

    template<typename T>
    bool set_text(QWidget* self, QString text)
    {
        auto obj = qobject_cast<T>(self);
        // Early return on Error. 
        if(obj == nullptr ){ return false;  }    
        obj->setText(text);
        return true;
    }

};


}

#endif // FORMLOADER_HPP
