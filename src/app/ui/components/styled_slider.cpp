// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "styled_slider.hpp"
#include "ui_theme.hpp"

#include <QMouseEvent>
#include <QToolTip>

namespace fluvel
{

StyledSlider::StyledSlider(QWidget* parent, ui::Appearance appearance)
    : JumpSlider(parent)
    , appearance_(appearance)
{
    if (appearance_ == ui::Appearance::Modern)
        setFixedHeight(sliderHeight());
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
        const int currentHandleRadius = hover_ ? handleHoverRadius() : handleRadius();

        painter.setPen(Qt::NoPen);

        painter.setBrush(grooveColor());
        painter.drawRoundedRect(groove, grooveHeight / 2, grooveHeight / 2);

        painter.setBrush(progressColor());
        painter.drawRoundedRect(progress, grooveHeight / 2, grooveHeight / 2);

        painter.setBrush(handleColor());
        painter.setPen(QPen(handleBorderColor(), 1));

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
    font.setPointSize(hoverFontPointSize());

    painter.setFont(font);

    const QMargins margins = hoverBubbleMargins();

    const QRect bubbleContentRect =
        QFontMetrics(font)
            .boundingRect(bubbleText_)
            .adjusted(-margins.left(), -margins.top(), margins.right(), margins.bottom());

    QRect bubbleRect = bubbleContentRect;
    bubbleRect.moveCenter(QPoint(hoverPosition_.x(), grooveRect().top() - hoverBubbleOffset()));

    const int x = std::clamp(bubbleRect.left(), 4, width() - bubbleRect.width() - 4);

    bubbleRect.moveLeft(x);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 20, 20, 180));
    painter.drawRoundedRect(bubbleRect, hoverBubbleRadius(), hoverBubbleRadius());

    painter.setPen(Qt::white);
    painter.drawText(bubbleRect, Qt::AlignCenter, bubbleText_);
}

QRect StyledSlider::grooveRect() const
{
    const int currentGrooveHeight = hover_ ? grooveHoverHeight() : grooveHeight();

    return QRect(ui::kSliderSideMargin,
                 topMargin() + (height() - topMargin() - bottomMargin() - currentGrooveHeight) / 2,
                 width() - 2 * ui::kSliderSideMargin, currentGrooveHeight);
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

QColor StyledSlider::grooveColor() const
{
    return QColor(255, 255, 255, 80);
}

QColor StyledSlider::progressColor() const
{
    return QColor(107, 111, 207);
}

QColor StyledSlider::handleColor() const
{
    return Qt::white;
}

QColor StyledSlider::handleBorderColor() const
{
    return QColor(107, 111, 207, 120);
}

int StyledSlider::grooveHeight() const
{
    return ui::kSliderGrooveHeight;
}

int StyledSlider::grooveHoverHeight() const
{
    return ui::kSliderGrooveHoverHeight;
}

int StyledSlider::handleRadius() const
{
    return ui::kSliderHandleRadius;
}

int StyledSlider::handleHoverRadius() const
{
    return ui::kSliderHandleHoverRadius;
}

int StyledSlider::topMargin() const
{
    return ui::kSliderTopMargin;
}

int StyledSlider::bottomMargin() const
{
    return ui::kSliderBottomMargin;
}

int StyledSlider::sliderHeight() const
{
    return ui::kSliderHeight;
}

int StyledSlider::hoverFontPointSize() const
{
    return 10;
}

QMargins StyledSlider::hoverBubbleMargins() const
{
    return {10, 4, 10, 4};
}

int StyledSlider::hoverBubbleRadius() const
{
    return 8;
}

int StyledSlider::hoverBubbleOffset() const
{
    return 22;
}

} // namespace fluvel