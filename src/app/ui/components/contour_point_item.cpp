#include "contour_point_item.hpp"
#include "application_settings.hpp"

#include <QPainter>

namespace ofeli_app
{

ContourPointsItem::ContourPointsItem()
{
    setTransformOriginPoint(0, 0);

    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
}

void ContourPointsItem::setPoints(const QVector<QPoint>& pts)
{
    prepareGeometryChange();
    points_ = pts;
}

void ContourPointsItem::clearPoints()
{
    prepareGeometryChange();
    points_.clear();
}

QRectF ContourPointsItem::boundingRect() const
{
    if (points_.isEmpty())
        return QRectF(0, 0, 1, 1);

    QRect r(points_.first(), QSize(1,1));
    for (const auto& p : points_)
        r |= QRect(p, QSize(1,1));

    QRectF rf(r);

    constexpr qreal margin = 5.0;
    rf.adjust(-margin, -margin, margin, margin);

    constexpr qreal minSize = 20.0;
    if (rf.width() < minSize)
        rf.setWidth(minSize);
    if (rf.height() < minSize)
        rf.setHeight(minSize);

    return rf;
}

void ContourPointsItem::paint(QPainter* painter,
                              const QStyleOptionGraphicsItem*,
                              QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, false);

    painter->setPen(color_);
    painter->setBrush(Qt::NoBrush);
    painter->drawPoints(points_.data(), points_.size());
}

}
