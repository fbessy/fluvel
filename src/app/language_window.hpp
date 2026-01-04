#ifndef LANGUAGE_WINDOW_HPP
#define LANGUAGE_WINDOW_HPP

#include "application_settings.hpp"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QListWidget;
QT_END_NAMESPACE

namespace ofeli_gui
{

class LanguageWindow : public QDialog
{

    Q_OBJECT

public :

    //! A parametric constructor with a pointer on the QWidget parent.
    LanguageWindow(QWidget* parent);

    //! Save window size and position.
    void apply_setting();

    //! Restores the current row of #list_widget from #selected_index.
    void cancel_setting();

private :

    virtual void closeEvent(QCloseEvent* event) override;

    Language& language;

    //! This widget is the part of the window that displays the list of languages.
    QListWidget* list_widget;
};

}

#endif // LANGUAGE_WINDOW_HPP


//! \class ofeli::LanguageWindow
//! The class LanguageWindow is a QDialog window to choose the application langage for the user. An instance of this class is created by #ofeli::MainWindow and displayed when the user clicks on menu Language.
