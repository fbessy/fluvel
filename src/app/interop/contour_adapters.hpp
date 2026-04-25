// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include <QPoint>
#include <QVector>

/**
 * @brief Convert a Fluvel contour to a Qt QVector of QPointF.
 *
 * Each discrete contour point (integer pixel coordinates) is converted
 * to continuous scene coordinates by mapping the pixel to its geometric center.
 *
 * In image space, a pixel (x, y) spans the region [x, x+1) × [y, y+1).
 * Its center is therefore located at (x + 0.5, y + 0.5).
 *
 * @param contour Input contour defined as discrete pixel coordinates.
 * @return QVector of QPointF representing the contour in continuous space.
 */
[[nodiscard]] inline QVector<QPointF> convertToQVector(const fluvel_ip::ExportedContour& contour)
{
    QVector<QPointF> q_contour;
    q_contour.reserve(static_cast<qsizetype>(contour.size()));

    // map pixel to its center (x + 0.5, y + 0.5)
    constexpr qreal kPixelCenterOffset = 0.5;

    for (const auto& point : contour)
    {
        q_contour.emplace_back(static_cast<qreal>(point.x) + kPixelCenterOffset,
                               static_cast<qreal>(point.y) + kPixelCenterOffset);
    }

    return q_contour;
}
