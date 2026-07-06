// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "overlay_text_item.hpp"

#include <QFontMetrics>
#include <QPainter>
#include <QtNumeric>

namespace
{
constexpr int kMaxTextWidth = 1000;
}

namespace fluvel
{

OverlayTextItem::OverlayTextItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    setAcceptedMouseButtons(Qt::LeftButton);

    QFont font;
    font.setStyleHint(QFont::SansSerif);

    // Enable tabular figures when supported by the font.
    font.setFeature("tnum", 1);

    font_ = font;
}

void OverlayTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setFont(font_);

    // Background
    painter->setBrush(backgroundColor_);
    painter->setPen(Qt::NoPen);

    const QRectF r = boundingRect().adjusted(0.5, 0.5, -0.5, -0.5);
    painter->drawRoundedRect(r, cornerRadius_, cornerRadius_);

    // Text
    painter->setPen(textColor_);

    painter->drawText(boundingRect().adjusted(padding_, padding_, -padding_, -padding_),
                      static_cast<int>(alignment_), text_);
}

const QFont& OverlayTextItem::font() const
{
    return font_;
}

void OverlayTextItem::setFont(const QFont& font)
{
    if (font_ == font)
        return;

    font_ = font;

    updateGeometry();
}

void OverlayTextItem::setAlignment(Qt::Alignment alignment)
{
    if (alignment_ == alignment)
        return;

    alignment_ = alignment;

    update();
}

void OverlayTextItem::setMinWidth(qreal width)
{
    if (qFuzzyCompare(minWidth_, width))
        return;

    minWidth_ = width;

    updateGeometry();
}

int OverlayTextItem::padding() const
{
    return padding_;
}

void OverlayTextItem::setPadding(int padding)
{
    if (padding_ == padding)
        return;

    padding_ = padding;

    updateGeometry();
}

qreal OverlayTextItem::cornerRadius() const
{
    return cornerRadius_;
}

void OverlayTextItem::setCornerRadius(qreal radius)
{
    if (qFuzzyCompare(cornerRadius_, radius))
        return;

    cornerRadius_ = radius;

    update();
}

const QColor& OverlayTextItem::backgroundColor() const
{
    return backgroundColor_;
}

void OverlayTextItem::setBackgroundColor(const QColor& color)
{
    if (backgroundColor_ == color)
        return;

    backgroundColor_ = color;

    update();
}

const QColor& OverlayTextItem::textColor() const
{
    return textColor_;
}

void OverlayTextItem::setTextColor(const QColor& color)
{
    if (textColor_ == color)
        return;

    textColor_ = color;

    update();
}

QRectF OverlayTextItem::boundingRect() const
{
    return rect_;
}

void OverlayTextItem::setText(const QString& text)
{
    if (text_ == text)
        return;

    text_ = text;

    updateGeometry();
}

void OverlayTextItem::updateGeometry()
{
    if (text_.isEmpty())
    {
        prepareGeometryChange();
        rect_ = QRectF{};
        update();
        return;
    }

    QFontMetrics fm(font_);

    QRect textRect =
        fm.boundingRect(QRect(0, 0, kMaxTextWidth, kMaxTextWidth), Qt::TextWordWrap, text_);

    prepareGeometryChange();

    const qreal width = textRect.width() + 2 * padding_;
    const qreal height = textRect.height() + 2 * padding_;

    rect_ = QRectF(0, 0, std::max(width, minWidth_), height);

    update();
}

} // namespace fluvel