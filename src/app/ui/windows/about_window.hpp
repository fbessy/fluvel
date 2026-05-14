// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QDialog>

class QWidget;
class QCloseEvent;

namespace fluvel_app
{

/**
 * @brief Dialog displaying information about the application.
 *
 * This dialog shows general information such as version, authors,
 * and license details. It may also provide links to external resources.
 */
class AboutWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the About dialog.
     *      * @param parent Optional parent widget.
     */
    AboutWindow(QWidget* parent = nullptr);

protected:
    /**
     * @brief Handles the dialog close event.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    void openHomepage();
    QString buildTechnicalSection();

    QDialog* licenseWindow_ = nullptr;
};

} // namespace fluvel_app
