// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QGraphicsItem>
#include <QPoint>
#include <QVector>

namespace fluvel_app
{

/**
 * @brief Graphics item displaying contour points.
 *
 * This item renders a set of 2D points as a contour overlay
 * on top of the image viewer.
 *
 * Points can be updated dynamically and are drawn using a
 * configurable color.
 */
class ContourPointsItem : public QGraphicsItem
{
public:
    /**
     * @brief Constructs the contour item.
     *      * @param parent Optional parent graphics item.
     */
    explicit ContourPointsItem(QGraphicsItem* parent = nullptr);

    /**
     * @name Data
     * @brief Manage contour points.
     * @{
     */

    /// Sets the contour points.
    void setPoints(const QVector<QPointF>& points);

    /// Clears all contour points.
    void clearPoints();

    /** @} */

    /**
     * @brief Sets the drawing color.
     *      * @param color Contour color.
     */
    void setColor(QColor color)
    {
        color_ = color;
        update();
    }

protected:
    /**
     * @name Rendering
     * @{
     */

    /// Returns the bounding rectangle of the contour.
    QRectF boundingRect() const override;

    /// Paints the contour points.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

    /** @} */

private:
    QVector<QPointF> points_;
    QColor color_ = Qt::red;
};

} // namespace fluvel_app
