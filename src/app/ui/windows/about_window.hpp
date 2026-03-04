// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QDialog>

namespace ofeli_app
{

class AboutWindow : public QDialog
{
    Q_OBJECT

public:
    //! A parametric constructor with a pointer on the QWidget parent.
    AboutWindow(QWidget* parent);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    //! A subwindow with the license application, displayed when the user clicks on the license
    //! button.
    QDialog* license_window_;

private slots:

    //! This function is called when the user clicks on web page button. It opens the
    //! application web site with the default web brower or a new tab of the web browser (if it
    //! is already opened).
    void open_webpage();
};

} // namespace ofeli_app

//! \class ofeli::AboutWindow
//! The class AboutWindow is a QDialog window that informs the user about Ofeli application. An
//! instance of this class is created by #ofeli::ImageWindow and displayed when the user clicks on
//! menu About.
