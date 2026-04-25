// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QDialog>

class QComboBox;
class QCloseEvent;
class QWidget;

namespace fluvel_app
{

/**
 * @brief Dialog for selecting the application language.
 *
 * This dialog allows the user to choose the UI language.
 * The selection is applied when the dialog is accepted and
 * restored when rejected.
 */
class LanguageWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the language selection dialog.
     *      * @param parent Optional parent widget.
     */
    LanguageWindow(QWidget* parent = nullptr);

protected:
    /**
     * @brief Applies the selected language to application settings.
     */
    void accept() override;

    /**
     * @brief Restores the language selection from current settings.
     */
    void reject() override;

    /**
     * @brief Handles dialog close events.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    QComboBox* combo_ = nullptr;
};

} // namespace fluvel_app
