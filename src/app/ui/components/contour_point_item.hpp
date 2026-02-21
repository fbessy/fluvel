#ifndef CONTOUR_POINT_ITEM_HPP
#define CONTOUR_POINT_ITEM_HPP

#include <QGraphicsItem>
#include <QVector>
#include <QPoint>

namespace ofeli_app {

class ContourPointsItem : public QGraphicsItem
{
public:
    explicit ContourPointsItem(QGraphicsItem* parent = nullptr);

    void setPoints(const QVector<QPointF>& points);
    void clearPoints();

    void setColor(QColor color) { color_ = color; update(); }

protected:
    QRectF boundingRect() const override;

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem*,
               QWidget*) override;

private:
    QVector<QPointF> points_;
    QColor color_ = Qt::red;
};

}

#endif // CONTOUR_POINT_ITEM_HPP
