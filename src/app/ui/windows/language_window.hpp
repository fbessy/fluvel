// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QDialog>

class QComboBox;
class QCloseEvent;
class QWidget;

namespace fluvel_app
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
    QComboBox* combo_ = nullptr;
};

} // namespace fluvel_app
