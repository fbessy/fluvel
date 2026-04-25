// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QIcon>
#include <QPushButton>
#include <QWidget>

namespace fluvel_app
{

/**
 * @brief Toggle button controlling the visibility of the right panel.
 *
 * This button updates its appearance (icon) depending on its checked state,
 * typically reflecting whether the right-side panel is visible or hidden.
 *
 * Internally switches between two icons:
 * - iconOn_ when checked
 * - iconOff_ when unchecked
 */
class RightPanelToggleButton : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the toggle button.
     *      * @param parent Optional parent widget.
     */
    explicit RightPanelToggleButton(QWidget* parent = nullptr);

private slots:
    /**
     * @brief Updates the button appearance based on its checked state.
     *      * Switches the displayed icon depending on whether the button is checked.
     *      * @param checked True if the button is active (panel visible), false otherwise.
     */
    void updateAppearance(bool checked);

private:
    /// Icon displayed when the button is checked (panel visible).
    QIcon iconOn_;

    /// Icon displayed when the button is unchecked (panel hidden).
    QIcon iconOff_;
};

} // namespace fluvel_app
