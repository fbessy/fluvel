// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include <QPoint>
#include <QVector>

[[nodiscard]] inline QVector<QPointF> convertToQVector(const fluvel_ip::ExportedContour& contour)
{
    QVector<QPointF> q_contour;
    q_contour.reserve(static_cast<qsizetype>(contour.size()));

    // Convert from discrete pixel indices (top-left corner convention)
    // to continuous scene coordinates centered on the pixel.
    // Pixel (x, y) spans [x, x+1) × [y, y+1) in image space.
    // Its geometric center is therefore at (x + 0.5, y + 0.5).

    constexpr qreal kPixelCenterOffset = 0.5;

    for (const auto& point : contour)
    {
        q_contour.emplace_back(static_cast<qreal>(point.x) + kPixelCenterOffset,
                               static_cast<qreal>(point.y) + kPixelCenterOffset);
    }

    return q_contour;
}
