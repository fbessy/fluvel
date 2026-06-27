// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "styled_slider.hpp"
#include "ui_theme.hpp"
#include <QMouseEvent>

namespace fluvel
{

StyledSlider::StyledSlider(QWidget* parent, ui::Appearance appearance)
    : JumpSlider(parent)
    , appearance_(appearance)
{
    if (appearance_ == ui::Appearance::Modern)
        setFixedHeight(ui::kSliderHeight);
}

void StyledSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (!isModernStyle())
    {
        JumpSlider::mouseMoveEvent(event);
        return;
    }

    hover_ = true;

    const double r = std::clamp(event->position().x() / double(width()), 0.0, 1.0);

    hoverText_ = hoverText(r);

    qDebug() << "hover =" << hoverText_;

    showHoverBubble_ = !hoverText_.isEmpty();

    hoverPosition_ = event->position();

    update();

    JumpSlider::mouseMoveEvent(event);
}

void StyledSlider::leaveEvent(QEvent* event)
{
    qDebug() << "leaveEvent";

    hover_ = false;
    showHoverBubble_ = false;

    update();

    JumpSlider::leaveEvent(event);
}

void StyledSlider::paintEvent(QPaintEvent* event)
{
    if (!isModernStyle())
    {
        QSlider::paintEvent(event);
        return;
    }

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    const QRect groove = grooveRect();
    const QRect progress = progressRect();
    const QPoint handle = handleCenter();

    const int grooveHeight = groove.height();
    const int handleRadius = hover_ ? 13 : 11;

    painter.setPen(Qt::NoPen);

    painter.setBrush(grooveColor());
    painter.drawRoundedRect(groove, grooveHeight / 2, grooveHeight / 2);

    painter.setBrush(progressColor());
    painter.drawRoundedRect(progress, grooveHeight / 2, grooveHeight / 2);

    painter.setBrush(handleColor());
    painter.setPen(QPen(handleBorderColor(), 1));

    painter.drawEllipse(handle, handleRadius, handleRadius);

    paintOverlay(painter);

    paintHoverBubble(painter);
}

void StyledSlider::paintHoverBubble(QPainter& painter)
{
    qDebug() << "paintHoverBubble" << showHoverBubble_ << hoverText_;

    if (!showHoverBubble_)
        return;

    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);

    const QRect textRect = QFontMetrics(font).boundingRect(hoverText_).adjusted(-10, -4, 10, 4);

    QRect bubbleRect = textRect;
    bubbleRect.moveCenter(QPoint(hoverPosition_.x(), grooveRect().top() - 22));

    const int x = std::clamp(bubbleRect.left(), 4, width() - bubbleRect.width() - 4);

    bubbleRect.moveLeft(x);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 20, 20, 180));
    painter.drawRoundedRect(bubbleRect, 8, 8);

    painter.setPen(Qt::white);
    painter.drawText(bubbleRect, Qt::AlignCenter, hoverText_);
}

QRect StyledSlider::grooveRect() const
{
    constexpr int kSideMargin = 16;
    constexpr int kTopMargin = 16;
    constexpr int kBottomMargin = 6;

    const int grooveHeight = hover_ ? 11 : 8;

    return QRect(kSideMargin,
                 kTopMargin + (height() - kTopMargin - kBottomMargin - grooveHeight) / 2,
                 width() - 2 * kSideMargin, grooveHeight);
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

} // namespace fluvel