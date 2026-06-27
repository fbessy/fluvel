// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "volume_slider.hpp"

#include <QMouseEvent>
#include <QToolTip>

namespace fluvel
{

VolumeSlider::VolumeSlider(QWidget* parent, ui::Appearance appearance)
    : StyledSlider(parent, appearance)
{
}

bool VolumeSlider::hasHoverBubble() const
{
    return true;
}

QString VolumeSlider::hoverText(double ratio) const
{
    const int volume = minimum() + ratio * (maximum() - minimum());

    return QString("%1 %").arg(volume);
}

} // namespace fluvel