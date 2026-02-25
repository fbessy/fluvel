#include "contour_point_item.hpp"
#include "application_settings.hpp"

#include <QPainter>

namespace ofeli_app
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
}

void ContourPointsItem::clearPoints()
{
    prepareGeometryChange();
    points_.clear();
}

QRectF ContourPointsItem::boundingRect() const
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

    constexpr qreal margin = 0.5;
    rf.adjust(-margin, -margin, margin, margin);

    return rf;
}

void ContourPointsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, false);

    painter->setPen(color_);
    painter->setBrush(Qt::NoBrush);

    painter->drawPoints(points_.data(), static_cast<int>(points_.size()));
}

} // namespace ofeli_app
