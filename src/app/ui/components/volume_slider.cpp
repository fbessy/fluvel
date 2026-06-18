// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "volume_slider.hpp"

#include <QMouseEvent>
#include <QToolTip>

#include <algorithm>

namespace fluvel
{

VolumeSlider::VolumeSlider(QWidget* parent)
    : JumpSlider(parent)
{
}

void VolumeSlider::mouseMoveEvent(QMouseEvent* event)
{
    const double ratio = std::clamp(static_cast<double>(event->position().x()) / width(), 0.0, 1.0);

    const int volume = minimum() + ratio * (maximum() - minimum());

    QToolTip::showText(event->globalPosition().toPoint(), QString("%1%").arg(volume), this);

    JumpSlider::mouseMoveEvent(event);
}

} // namespace fluvel