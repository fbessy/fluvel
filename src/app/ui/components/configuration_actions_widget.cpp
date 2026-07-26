// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "configuration_actions_widget.hpp"

#include "animated_push_button.hpp"
#include "icon_loader.hpp"
#include "right_panel_toggle_button.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>

namespace fluvel
{

ConfigurationActionsWidget::ConfigurationActionsWidget(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    displayPanelButton_ = new RightPanelToggleButton(this);

    configurationMenuButton_ = createButton("configure", ":/icons/actions/settings-symbolic.svg",
                                            tr("Open configuration menu."));

    configurationMenu_ = new QMenu(this);

    sessionSettingsAction_ = configurationMenu_->addAction(tr("Session Settings..."));

    configurationMenu_->addSeparator();

    preferencesAction_ = configurationMenu_->addAction(tr("Preferences..."));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(displayPanelButton_);
    layout->addWidget(configurationMenuButton_);

    connect(displayPanelButton_, &QPushButton::toggled, this,
            &ConfigurationActionsWidget::displayPanelToggled);

    connect(sessionSettingsAction_, &QAction::triggered, this,
            &ConfigurationActionsWidget::sessionSettingsRequested);

    connect(preferencesAction_, &QAction::triggered, this,
            &ConfigurationActionsWidget::preferencesRequested);

    connect(configurationMenuButton_, &QPushButton::clicked, this,
            [this]
            {
                const QPoint pos = configurationMenuButton_->mapToGlobal(
                    QPoint(0, configurationMenuButton_->height()));

                configurationMenu_->exec(pos);
            });
}

AnimatedPushButton* ConfigurationActionsWidget::createButton(const QString& iconName,
                                                             const QString& fallbackIcon,
                                                             const QString& toolTip)
{
    auto* button = new AnimatedPushButton(this);

    button->setFlat(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(toolTip);
    button->setIcon(il::loadIcon(iconName, fallbackIcon));

    return button;
}

} // namespace fluvel