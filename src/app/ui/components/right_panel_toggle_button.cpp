// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "right_panel_toggle_button.hpp"
#include "icon_loader.hpp"

#include <QPushButton>

namespace fluvel
{

RightPanelToggleButton::RightPanelToggleButton(QWidget* parent)
    : AnimatedPushButton(parent)
    , iconOn_(il::loadIcon(":/icons/view/sidebar-right-show.svg"))
    , iconOff_(il::loadIcon(":/icons/view/sidebar-right-hide.svg"))
{
    setCheckable(true);
    setChecked(true);
    setFocusPolicy(Qt::NoFocus);

    setTransitionEffect(TransitionEffect::Slide);
    setClickAnimation(ClickAnimation::None);

    // Initial state
    updateAppearance(isChecked());

    connect(this, &QPushButton::toggled, this, &RightPanelToggleButton::updateAppearance);
}

void RightPanelToggleButton::updateAppearance(bool checked)
{
    if (checked)
        setAnimatedIcon(iconOn_, TransitionDirection::Left);
    else
        setAnimatedIcon(iconOff_, TransitionDirection::Right);

    setToolTip(checked ? tr("Hide right panel.") : tr("Show right panel."));
}

} // namespace fluvel
