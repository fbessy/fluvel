// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "right_panel_toggle_button.hpp"

namespace ofeli_app
{

RightPanelToggleButton::RightPanelToggleButton(QWidget* parent)
    : QPushButton(parent)
    , iconOn_(":/icons/toolbar/right_panel_on.svg")
    , iconOff_(":/icons/toolbar/right_panel_off.svg")
{
    setCheckable(true);
    setChecked(true);
    setFocusPolicy(Qt::NoFocus);

    // Initial state
    updateAppearance(isChecked());

    connect(this, &QPushButton::toggled, this, &RightPanelToggleButton::updateAppearance);
}

void RightPanelToggleButton::updateAppearance(bool checked)
{
    setIcon(checked ? iconOn_ : iconOff_);

    setToolTip(checked ? tr("Right panel is visible.") : tr("Right panel is hidden."));
}

} // namespace ofeli_app
