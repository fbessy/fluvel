// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "timeline_slider.hpp"
#include "time_utils.hpp"

#include <QMouseEvent>
#include <QToolTip>

#include <algorithm>

namespace fluvel
{

TimelineSlider::TimelineSlider(QWidget* parent)
    : JumpSlider(parent)
{
}

void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
{
    const double ratio = std::clamp(static_cast<double>(event->position().x()) / width(), 0.0, 1.0);

    const qint64 positionMs = minimum() + ratio * (maximum() - minimum());

    QToolTip::showText(event->globalPosition().toPoint(), time_utils::formatDuration(positionMs),
                       this);

    QSlider::mouseMoveEvent(event);
}

} // namespace fluvel