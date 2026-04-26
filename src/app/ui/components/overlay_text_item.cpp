// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "overlay_text_item.hpp"

namespace fluvel_app
{

OverlayTextItem::OverlayTextItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    setAcceptedMouseButtons(Qt::LeftButton);

    QFont font; // police par défaut
    font.setStyleHint(QFont::SansSerif);

    // active les chiffres tabulaires
    font.setFeature("tnum", 1);

    font_ = font;
}

void OverlayTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setFont(font_);

    // Fond semi-transparent
    painter->setBrush(QColor(0, 0, 0, 180));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect(), 5, 5);

    // Texte
    painter->setPen(Qt::white);
    painter->drawText(boundingRect().adjusted(8, 6, -8, -6), alignment_, text_);
}

void OverlayTextItem::setAlignment(Qt::Alignment align)
{
    alignment_ = align;
    update(); // redraw
}

void OverlayTextItem::setMinWidth(qreal w)
{
    minWidth_ = w;
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

    QFontMetrics fm(font_);

    QRect textRect = fm.boundingRect(QRect(0, 0, 1000, 1000), // zone large
                                     Qt::TextWordWrap, text_);

    prepareGeometryChange();

    qreal width = textRect.width() + 2 * padding_;
    qreal height = textRect.height() + 2 * padding_;

    rect_ = QRectF(0, 0, std::max(width, minWidth_), height);

    update();
}

} // namespace fluvel_app
