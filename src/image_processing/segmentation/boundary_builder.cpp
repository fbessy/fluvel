// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "boundary_builder.hpp"

#include "fluvel_math.hpp"
#include "point_containers.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

namespace fluvel_ip
{

ContourPointsSet BoundaryBuilder::generateEllipse(int width, int height, float widthRatio,
                                                  float heightRatio)
{
    ContourPointsSet contourSet;

    contourSet.gridWidth = width;
    contourSet.gridHeight = height;

    BoundaryBuilder builder(width, height, contourSet.outer, contourSet.inner);

    builder.generateEllipsePoints(widthRatio, heightRatio);

    return contourSet;
}

ContourPointsSet BoundaryBuilder::generateRectangle(int width, int height, Point2D_f topLeft,
                                                    Point2D_f bottomRight)
{
    ContourPointsSet contourSet;

    contourSet.gridWidth = width;
    contourSet.gridHeight = height;

    BoundaryBuilder builder(width, height, contourSet.outer, contourSet.inner);

    builder.generateRectanglePoints(topLeft, bottomRight);

    return contourSet;
}

BoundaryBuilder::BoundaryBuilder(int gridWidth, int gridHeight, ContourPoints& outerBoundary,
                                 ContourPoints& innerBoundary)
    : gridWidth_(gridWidth)
    , gridHeight_(gridHeight)
    , outerBoundary_(outerBoundary)
    , innerBoundary_(innerBoundary)
{
    assert(gridWidth >= 1);
    assert(gridHeight >= 1);

    const size_t perimeter = static_cast<size_t>(2 * (gridWidth + gridHeight));

    const size_t reserveSize = 3 * perimeter;

    outerBoundary_.reserve(reserveSize);
    innerBoundary_.reserve(reserveSize);
}

void BoundaryBuilder::generateRectanglePoints(Point2D_i topLeft, Point2D_i bottomRight,
                                              BoundaryOrientation orientation)
{
    auto* outerBoundary = &outerBoundary_;
    auto* innerBoundary = &innerBoundary_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(outerBoundary, innerBoundary);
    }

    generateRectanglePoints(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y, *outerBoundary,
                            *innerBoundary);
}

void BoundaryBuilder::generateRectanglePoints(Point2D_f topLeft, Point2D_f bottomRight,
                                              BoundaryOrientation orientation)
{
    auto* outerBoundary = &outerBoundary_;
    auto* innerBoundary = &innerBoundary_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(outerBoundary, innerBoundary);
    }

    generateRectanglePoints(
        std::lround(topLeft.x * gridWidth_), std::lround(topLeft.y * gridHeight_),
        std::lround(bottomRight.x * gridWidth_), std::lround(bottomRight.y * gridHeight_),
        *outerBoundary, *innerBoundary);
}

void BoundaryBuilder::generateRectanglePoints(int x1, int y1, int x2, int y2,
                                              ContourPoints& outerBoundary,
                                              ContourPoints& innerBoundary)
{
    if (x1 > x2)
    {
        std::swap(x1, x2);
    }
    if (y1 > y2)
    {
        std::swap(y1, y2);
    }

    if (x1 != x2 && y1 != y2)
    {
        generateRectanglePointsForOneList(innerBoundary, x1, y1, x2, y2);

#ifdef ALGO_8_CONNEXITY
        generateRectanglePointsForOneList(outerBoundary, x1 - 1, y1 - 1, x2 + 1, y2 + 1);
#else
        for (int x = x1; x <= x2; ++x)
        {
            if (x >= 0 && x < gridWidth_)
            {
                if (y1 - 1 >= 0 && y1 - 1 < gridHeight_)
                    outerBoundary.emplace_back(x, y1 - 1);

                if (y2 + 1 >= 0 && y2 + 1 < gridHeight_)
                    outerBoundary.emplace_back(x, y2 + 1);
            }
        }

        for (int y = y1; y <= y2; ++y)
        {
            if (y >= 0 && y < gridHeight_)
            {
                if (x1 - 1 >= 0 && x1 - 1 < gridWidth_)
                    outerBoundary.emplace_back(x1 - 1, y);

                if (x2 + 1 >= 0 && x2 + 1 < gridWidth_)
                    outerBoundary.emplace_back(x2 + 1, y);
            }
        }
#endif
    }
}

void BoundaryBuilder::generateRectanglePointsForOneList(ContourPoints& boundary, int x1, int y1,
                                                        int x2, int y2)
{
    for (int x = x1; x <= x2; ++x)
    {
        if (x >= 0 && x < gridWidth_)
        {
            if (y1 >= 0 && y1 < gridHeight_)
                boundary.emplace_back(x, y1);

            if (y2 >= 0 && y2 < gridHeight_)
                boundary.emplace_back(x, y2);
        }
    }

    for (int y = y1 + 1; y < y2; ++y)
    {
        if (y >= 0 && y < gridHeight_)
        {
            if (x1 >= 0 && x1 < gridWidth_)
                boundary.emplace_back(x1, y);

            if (x2 >= 0 && x2 < gridWidth_)
                boundary.emplace_back(x2, y);
        }
    }
}

void BoundaryBuilder::generateEllipsePoints(int width, int height, Point2D_i center,
                                            BoundaryOrientation orientation)
{
    auto* outerBoundary = &outerBoundary_;
    auto* innerBoundary = &innerBoundary_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(outerBoundary, innerBoundary);
    }

    const int a = width / 2;
    const int b = height / 2;

    generateEllipsePoints(center.x, center.y, a, b, *outerBoundary, *innerBoundary);
}

void BoundaryBuilder::generateEllipsePoints(float widthRatio, float heightRatio, Point2D_f center,
                                            BoundaryOrientation orientation)
{
    auto* outerBoundary = &outerBoundary_;
    auto* innerBoundary = &innerBoundary_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(outerBoundary, innerBoundary);
    }

    const float cx = (center.x + 0.5f) * gridWidth_;
    const float cy = (center.y + 0.5f) * gridHeight_;

    const int x0 = std::lround(cx);
    const int y0 = std::lround(cy);

    const int a = std::lround((widthRatio / 2.f) * gridWidth_);
    const int b = std::lround((heightRatio / 2.f) * gridHeight_);

    generateEllipsePoints(x0, y0, a, b, *outerBoundary, *innerBoundary);
}

void BoundaryBuilder::generateEllipsePoints(int x0, int y0, int a, int b,
                                            ContourPoints& outerBoundary,
                                            ContourPoints& innerBoundary)
{
    buildEllipseMidpointConnected(x0, y0, a, b, outerBoundary);
    buildInnerContiguous(x0, y0, outerBoundary, innerBoundary);
}

void BoundaryBuilder::buildEllipseMidpointConnected(int x0, int y0, int a, int b,
                                                    ContourPoints& outerBoundary)
{
    int x = 0;
    int y = b;

    // Carrés
    int64_t a2 = static_cast<int64_t>(a * a);
    int64_t b2 = static_cast<int64_t>(b * b);

    int64_t dx = 0;
    int64_t dy = 2 * a2 * y;

    // Paramètre de décision région 1
    int64_t d1 = b2 - (a2 * b) + (a2 / 4);

    int prev_x = x;
    int prev_y = y;

    // -------- Région 1 --------
    while (dx < dy)
    {
        // Ajout du point principal
        add4pointsInEllipse(outerBoundary, x, y, x0, y0);

        // ---- Connexité 8 ----
        int dxp = x - prev_x;
        int dyp = y - prev_y;

        if (abs(dxp) == 1 && abs(dyp) == 1)
        {
            // pixel correcteur (choix simple et sûr)
            add4pointsInEllipse(outerBoundary, prev_x, y, x0, y0);
        }

        prev_x = x;
        prev_y = y;

        ++x;
        dx += (2 * b2);

        if (d1 < 0)
        {
            d1 += (dx + b2);
        }
        else
        {
            --y;
            dy -= (2 * a2);
            d1 += (dx - dy + b2);
        }
    }

    // Paramètre région 2
    const int64_t x_shift = static_cast<int64_t>(x) * 2 + 1; // 2*(x + 0.5)
    const int64_t y_shift = static_cast<int64_t>(y) - 1;

    int64_t d2 = (b2 * x_shift * x_shift / 4) + (a2 * y_shift * y_shift) - (a2 * b2);

    // -------- Région 2 --------
    while (y >= 0)
    {
        add4pointsInEllipse(outerBoundary, x, y, x0, y0);

        // ---- Connexité 8 ----
        int dxp = x - prev_x;
        int dyp = y - prev_y;

        if (std::abs(dxp) == 1 && std::abs(dyp) == 1)
        {
            add4pointsInEllipse(outerBoundary, x, prev_y, x0, y0);
        }

        prev_x = x;
        prev_y = y;

        --y;
        dy -= (2 * a2);

        if (d2 > 0)
        {
            d2 += (a2 - dy);
        }
        else
        {
            ++x;
            dx += (2 * b2);
            d2 += (dx - dy + a2);
        }
    }
}

void BoundaryBuilder::buildInnerContiguous(int x0, int y0, const ContourPoints& outerBoundary,
                                           ContourPoints& innerBoundary)
{
    PointSet seen;
    seen.reserve(outerBoundary.size());

    for (const auto& point : outerBoundary)
    {
        const int sx = math::sign(x0 - point.x());
        const int sy = math::sign(y0 - point.y());

        const Point2D_i candidatePoint{point.x() + sx, point.y() + sy};

        if (!insideGrid(candidatePoint))
            continue;

        if (seen.insert(candidatePoint).second)
            innerBoundary.emplace_back(candidatePoint);
    }
}

void BoundaryBuilder::checkDuplicates(const ContourPoints& contour)
{
    PointSet seen;
    seen.reserve(contour.size());

    for (const auto& contourPoint : contour)
    {
        const Point2D_i point{contourPoint.x(), contourPoint.y()};

        if (!seen.insert(point).second)
        {
            std::cerr << " pos=(" << point.x << "," << point.y << ")\n";
            std::cerr.flush();
        }
    }
}

} // namespace fluvel_ip
