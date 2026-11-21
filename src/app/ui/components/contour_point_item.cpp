// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "contour_point_item.hpp"

#include <QPainter>

namespace
{
// Width of the pen used to draw the contour points, in image pixels
// (the pen is not cosmetic, so it is scaled with the zoom).
constexpr qreal kPenWidth = 1.1;
} // namespace

namespace fluvel
{

ContourPointsItem::ContourPointsItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setTransformOriginPoint(0.0, 0.0);

    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
}

void ContourPointsItem::setPoints(const QVector<QPointF>& points)
{
    prepareGeometryChange();
    points_ = points;
    boundingRect_ = computeBoundingRect();
}

void ContourPointsItem::clearPoints()
{
    prepareGeometryChange();
    points_.clear();
    boundingRect_ = computeBoundingRect();
}

QRectF ContourPointsItem::boundingRect() const
{
    // Cached: the rectangle only changes with the points, and the scene
    // queries it on every update.
    return boundingRect_;
}

QRectF ContourPointsItem::computeBoundingRect() const
{
    if (points_.isEmpty())
        return QRectF(0.0, 0.0, 1.0, 1.0);

    qreal minX = points_.first().x();
    qreal maxX = minX;
    qreal minY = points_.first().y();
    qreal maxY = minY;

    for (const auto& p : points_)
    {
        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }

    QRectF rf(minX, minY, maxX - minX, maxY - minY);

    // Pixel (x, y) spans [x, x+1) × [y, y+1) in image space.
    // Its geometric center is therefore at (x + 0.5, y + 0.5).
    //
    // The bounding rectangle must also contain the whole stroke of the
    // outermost points: half of the pen width goes beyond the pixel.

    constexpr qreal margin = 0.5 + kPenWidth;
    rf.adjust(-margin, -margin, margin, margin);

    return rf;
}

void ContourPointsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->setRenderHint(QPainter::TextAntialiasing, false);

    QPen pen(color_);
    pen.setWidthF(kPenWidth);
    pen.setCosmetic(false);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawPoints(points_.data(), static_cast<int>(points_.size()));
}

} // namespace fluvel
