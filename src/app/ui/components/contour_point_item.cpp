#include "contour_point_item.hpp"

#include <QPainter>

namespace ofeli_app
{

void ContourPointsItem::setPoints(const QVector<QPoint>& pts)
{
    prepareGeometryChange();
    points_ = pts;
}

QRectF ContourPointsItem::boundingRect() const
{
    if (points_.isEmpty())
        return QRectF();

    QRect r(points_.first(), QSize(1,1));
    for (const auto& p : points_)
        r |= QRect(p, QSize(1,1));
    return r;
}

void ContourPointsItem::paint(QPainter* painter,
                              const QStyleOptionGraphicsItem*,
                              QWidget*)
{
    painter->setPen(color_);
    painter->setBrush(Qt::NoBrush);
    painter->drawPoints(points_.data(), points_.size());
}

}
