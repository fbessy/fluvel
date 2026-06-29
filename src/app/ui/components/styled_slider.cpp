// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "styled_slider.hpp"
#include "ui_theme.hpp"

#include <QMouseEvent>
#include <QToolTip>

namespace fluvel
{

StyledSlider::StyledSlider(QWidget* parent, ui::Appearance appearance, SliderStyle style)
    : JumpSlider(parent)
    , style_(style)
    , appearance_(appearance)
{
    if (appearance_ == ui::Appearance::Modern)
        setFixedHeight(style_.sliderHeight);
}

void StyledSlider::mouseMoveEvent(QMouseEvent* event)
{
    hover_ = true;

    const double r = std::clamp(event->position().x() / double(width()), 0.0, 1.0);

    bubbleText_ = hoverText(r);
    hoverPosition_ = event->position();

    if (isModernStyle())
    {
        showHoverBubble_ = !bubbleText_.isEmpty();
        update();
    }
    else
    {
        if (bubbleText_.isEmpty())
            QToolTip::hideText();
        else
            QToolTip::showText(event->globalPosition().toPoint(), bubbleText_, this);
    }

    JumpSlider::mouseMoveEvent(event);
}

void StyledSlider::leaveEvent(QEvent* event)
{
    hover_ = false;
    showHoverBubble_ = false;

    if (isModernStyle())
        update();
    else
        QToolTip::hideText();

    JumpSlider::leaveEvent(event);
}

void StyledSlider::paintEvent(QPaintEvent* event)
{
    if (isModernStyle())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRect groove = grooveRect();
        const QRect progress = progressRect();
        const QPoint handle = handleCenter();

        const int grooveHeight = groove.height();
        const int currentHandleRadius = hover_ ? style_.handleHoverRadius : style_.handleRadius;

        painter.setPen(Qt::NoPen);

        painter.setBrush(ui::kSliderGrooveColor);
        painter.drawRoundedRect(groove, grooveHeight / 2, grooveHeight / 2);

        painter.setBrush(ui::kSliderProgressColor);
        painter.drawRoundedRect(progress, grooveHeight / 2, grooveHeight / 2);

        painter.setBrush(ui::kSliderHandleColor);
        painter.setPen(QPen(ui::kSliderHandleBorderColor, 1));

        painter.drawEllipse(handle, currentHandleRadius, currentHandleRadius);

        paintOverlay(painter);
        paintHoverBubble(painter);
    }
    else
    {
        QSlider::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        paintOverlay(painter);
    }
}

void StyledSlider::paintHoverBubble(QPainter& painter)
{
    if (!showHoverBubble_ || bubbleText_.isEmpty())
        return;

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(style_.hoverFontPointSize);

    painter.setFont(font);

    const QMargins margins = style_.hoverBubbleMargins;

    const QRect bubbleContentRect =
        QFontMetrics(font)
            .boundingRect(bubbleText_)
            .adjusted(-margins.left(), -margins.top(), margins.right(), margins.bottom());

    QRect bubbleRect = bubbleContentRect;
    bubbleRect.moveCenter(
        QPoint(hoverPosition_.x(), grooveRect().top() - style_.hoverBubbleOffset));

    const int x = std::clamp(bubbleRect.left(), 4, width() - bubbleRect.width() - 4);

    bubbleRect.moveLeft(x);

    painter.setPen(Qt::NoPen);
    painter.setBrush(ui::kTooltipBackgroundColor);
    painter.drawRoundedRect(bubbleRect, style_.hoverBubbleRadius, style_.hoverBubbleRadius);

    painter.setPen(ui::kTooltipTextColor);
    painter.drawText(bubbleRect, Qt::AlignCenter, bubbleText_);
}

QRect StyledSlider::grooveRect() const
{
    const int currentGrooveHeight = hover_ ? style_.grooveHoverHeight : style_.grooveHeight;

    return QRect(style_.sideMargin,
                 style_.topMargin +
                     (height() - style_.topMargin - style_.bottomMargin - currentGrooveHeight) / 2,
                 width() - 2 * style_.sideMargin, currentGrooveHeight);
}

double StyledSlider::ratio() const
{
    if (maximum() == minimum())
        return 0.0;

    return double(value() - minimum()) / double(maximum() - minimum());
}

QRect StyledSlider::progressRect() const
{
    QRect r = grooveRect();

    r.setWidth(int(r.width() * ratio()));

    return r;
}

QPoint StyledSlider::handleCenter() const
{
    QRect groove = grooveRect();

    return QPoint(groove.left() + int(groove.width() * ratio()), groove.center().y());
}

bool StyledSlider::isModernStyle() const
{
    return appearance_ == ui::Appearance::Modern;
}

} // namespace fluvel