// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QWidget>

class QHBoxLayout;
class QAction;
class QMenu;

namespace fluvel
{

class RightPanelToggleButton;
class AnimatedPushButton;

/**
 * @brief Widget providing configuration-related actions.
 *
 * This widget gathers the buttons used to configure the current session and
 * the application. It provides a consistent appearance and behavior across
 * the different application windows while remaining independent from the
 * dialogs they open.
 *
 * The widget does not directly manipulate any dialog. Instead, it emits
 * signals that can be connected to the appropriate dialogs by the parent
 * window.
 */
class ConfigurationActionsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the configuration actions widget.
     * @param parent Parent widget.
     */
    explicit ConfigurationActionsWidget(QWidget* parent = nullptr);

signals:

    /**
     * @brief Emitted when the display panel visibility is toggled.
     * @param checked True if the panel should be visible.
     */
    void displayPanelToggled(bool checked);

    /**
     * @brief Emitted when the session settings button is clicked.
     */
    void sessionSettingsRequested();

    /**
     * @brief Emitted when the application preferences button is clicked.
     */
    void preferencesRequested();

private:
    /**
     * @brief Creates a configuration button.
     *
     * The returned button is initialized with the common appearance used by
     * configuration actions (flat style, no focus, icon and tooltip).
     *
     * @param iconName Theme icon name.
     * @param fallbackIcon Fallback icon resource.
     * @param toolTip Button tooltip.
     * @return Newly created button.
     */
    AnimatedPushButton* createButton(const QString& iconName, const QString& fallbackIcon,
                                     const QString& toolTip);

    RightPanelToggleButton* displayPanelButton_{nullptr};
    AnimatedPushButton* configurationMenuButton_{nullptr};
    QMenu* configurationMenu_{nullptr};
    QAction* sessionSettingsAction_{nullptr};
    QAction* preferencesAction_{nullptr};
};

} // namespace fluvel