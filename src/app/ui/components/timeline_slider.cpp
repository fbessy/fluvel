// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "timeline_slider.hpp"
#include "time_utils.hpp"

#include <QMouseEvent>
#include <QToolTip>
#include <QWidget>

namespace fluvel
{

TimelineSlider::TimelineSlider(QWidget* parent)
    : QSlider(Qt::Horizontal, parent)
{
    setMouseTracking(true);
}

void TimelineSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const double ratio = static_cast<double>(event->position().x()) / width();

        setValue(minimum() + ratio * (maximum() - minimum()));
    }

    QSlider::mousePressEvent(event);
}

void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
{
    const double ratio = static_cast<double>(event->position().x()) / width();

    const qint64 positionMs = minimum() + ratio * (maximum() - minimum());

    QToolTip::showText(event->globalPosition().toPoint(), time_utils::formatDuration(positionMs),
                       this);

    QSlider::mouseMoveEvent(event);
}

} // namespace fluvel