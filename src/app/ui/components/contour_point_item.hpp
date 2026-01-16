#ifndef CONTOUR_POINT_ITEM_HPP
#define CONTOUR_POINT_ITEM_HPP

#include <QGraphicsItem>
#include <QVector>
#include <QPoint>

namespace ofeli_app {

class ContourPointsItem : public QGraphicsItem
{
public:
    ContourPointsItem();

    void setPoints(const QVector<QPoint>& pts);
    void clearPoints();


    QRectF boundingRect() const override;

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem*,
               QWidget*) override;

    void setColor(QColor color) { color_ = color; }

private:
    QVector<QPoint> points_;
    QColor color_ = Qt::red;
};

}

#endif // CONTOUR_POINT_ITEM_HPP
