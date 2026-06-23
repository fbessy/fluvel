// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "timeline_slider.hpp"
#include "time_utils.hpp"

#include <QMouseEvent>
#include <QToolTip>

#include <algorithm>

namespace fluvel
{

TimelineSlider::TimelineSlider(QWidget* parent, bool fullscreenStyle)
    : JumpSlider(parent)
    , fullscreenStyle_(fullscreenStyle)
{
}

bool TimelineSlider::isFullscreenStyle() const
{
    return fullscreenStyle_;
}

void TimelineSlider::paintEvent(QPaintEvent* event)
{
    if (!fullscreenStyle_)
    {
        QSlider::paintEvent(event);
        return;
    }

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    constexpr int grooveHeight = 9;
    constexpr int handleRadius = 12;

    QRect grooveRect(12, (height() - grooveHeight) / 2, width() - 24, grooveHeight);

    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(255, 255, 255, 80));

    painter.drawRoundedRect(grooveRect, grooveHeight / 2, grooveHeight / 2);

    double ratio = 0.0;

    if (maximum() > minimum())
    {
        ratio = double(value() - minimum()) / double(maximum() - minimum());
    }

    QRect progressRect = grooveRect;

    progressRect.setWidth(grooveRect.width() * ratio);

    painter.setBrush(QColor(107, 111, 207));

    painter.drawRoundedRect(progressRect, grooveHeight / 2, grooveHeight / 2);

    const int handleX = grooveRect.left() + grooveRect.width() * ratio;

    painter.setBrush(Qt::white);

    painter.drawEllipse(QPoint(handleX, grooveRect.center().y()), handleRadius, handleRadius);

    if (showHoverTime_)
    {
        QFont font = painter.font();

        font.setBold(true);

        painter.setFont(font);

        const QRect textRect = QFontMetrics(font).boundingRect(hoverText_).adjusted(-10, -4, 10, 4);

        QRect bubbleRect = textRect;

        bubbleRect.moveCenter(QPoint(hoverPosition_.x(), grooveRect.top() - 22));

        const int x = std::clamp(bubbleRect.left(), 4, width() - bubbleRect.width() - 4);

        bubbleRect.moveLeft(x);

        painter.setPen(Qt::NoPen);

        painter.setBrush(QColor(20, 20, 20, 180));

        painter.drawRoundedRect(bubbleRect, 8, 8);

        painter.setPen(Qt::white);

        painter.drawText(bubbleRect, Qt::AlignCenter, hoverText_);
    }
}

/*void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
{
    const double ratio = std::clamp(static_cast<double>(event->position().x()) / width(), 0.0, 1.0);

    const qint64 positionMs = minimum() + ratio * (maximum() - minimum());

    QToolTip::showText(event->globalPosition().toPoint(), time_utils::formatDuration(positionMs),
                       this);

    QSlider::mouseMoveEvent(event);
}*/

void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
{
    const double ratio = std::clamp(static_cast<double>(event->position().x()) / width(), 0.0, 1.0);

    const qint64 positionMs = minimum() + ratio * (maximum() - minimum());

    const QString text = time_utils::formatDuration(positionMs);

    if (fullscreenStyle_)
    {
        hoverText_ = text;

        hoverPosition_ = event->position();

        showHoverTime_ = true;

        update();
    }
    else
    {
        QToolTip::showText(event->globalPosition().toPoint(), text, this);
    }

    QSlider::mouseMoveEvent(event);
}

void TimelineSlider::leaveEvent(QEvent* event)
{
    if (fullscreenStyle_)
    {
        showHoverTime_ = false;

        update();
    }

    QWidget::leaveEvent(event);
}

} // namespace fluvel