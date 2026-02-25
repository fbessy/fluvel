#ifndef LANGUAGE_WINDOW_HPP
#define LANGUAGE_WINDOW_HPP

#include <QDialog>

class QComboBox;

namespace ofeli_app
{

class LanguageWindow : public QDialog
{
    Q_OBJECT

public:
    //! A parametric constructor with a pointer on the QWidget parent.
    LanguageWindow(QWidget* parent);

protected:
    //! Save the language chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the language combobox state in function of the ApplicationSettings language.
    void reject() override;

    void closeEvent(QCloseEvent* event) override;

private:
    QComboBox* combo_;
};

} // namespace ofeli_app

#endif // LANGUAGE_WINDOW_HPP

//! \class ofeli::LanguageWindow
//! The class LanguageWindow is a QDialog window to choose the application langage for the user. An
//! instance of this class is created by #ofeli::ImageWindow and displayed when the user clicks on
//! menu Language.
