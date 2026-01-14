#ifndef CONTOUR_POINT_ITEM_HPP
#define CONTOUR_POINT_ITEM_HPP

#include <QGraphicsItem>
#include <QVector>
#include <QPoint>

namespace ofeli_app {

class ContourPointsItem : public QGraphicsItem
{
public:
    void setPoints(const QVector<QPoint>& pts);
    void setColor(QColor color) { color_ = color; }

    QRectF boundingRect() const override;

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem*,
               QWidget*) override;

private:
    QVector<QPoint> points_;
    QColor color_ = Qt::red;
};

}

#endif // CONTOUR_POINT_ITEM_HPP
