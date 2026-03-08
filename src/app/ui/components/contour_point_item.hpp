// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QGraphicsItem>
#include <QPoint>
#include <QVector>

namespace fluvel_app
{

class ContourPointsItem : public QGraphicsItem
{
public:
    explicit ContourPointsItem(QGraphicsItem* parent = nullptr);

    void setPoints(const QVector<QPointF>& points);
    void clearPoints();

    void setColor(QColor color)
    {
        color_ = color;
        update();
    }

protected:
    QRectF boundingRect() const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    QVector<QPointF> points_;
    QColor color_ = Qt::red;
};

} // namespace fluvel_app
