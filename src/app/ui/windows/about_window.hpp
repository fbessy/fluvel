// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QDialog>

class QWidget;
class QCloseEvent;

namespace fluvel_app
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
    void openWebPage();

    QDialog* licenseWindow_ = nullptr;
};

} // namespace fluvel_app
