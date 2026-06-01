// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include "point.hpp"

namespace fluvel_ip
{

/**
 * @brief Defines the orientation of the generated boundary.
 *
 * This affects the ordering of points in the generated contour,
 * which can be important for algorithms relying on orientation
 * (e.g. inward/outward propagation).
 */
enum class BoundaryOrientation
{
    Normal,  ///< Standard orientation (default)
    Reversed ///< Reversed point ordering
};

/**
 * @brief Utility class to generate initial contours (boundaries).
 *
 * This class builds discrete boundary representations (outer and inner contours)
 * for geometric shapes such as rectangles and ellipses, constrained within a grid.
 *
 * It is typically used to initialize active contour algorithms with a valid
 * starting shape.
 */
class BoundaryFactory
{
public:
    /**
     * @brief Generate an elliptical contour set.
     *
     * Creates the outer and inner boundaries of an ellipse defined
     * on a grid of the specified size.
     *
     * @param width Grid width.
     * @param height Grid height.
     * @param widthRatio Ellipse width ratio relative to the grid width.
     * @param heightRatio Ellipse height ratio relative to the grid height.
     *
     * @return Generated contour set.
     */
    static ContourPointsSet generateEllipse(int width, int height, float widthRatio,
                                            float heightRatio);

    /**
     * @brief Generate a rectangular contour set.
     *
     * Creates the outer and inner boundaries of a rectangle defined
     * by normalized coordinates.
     *
     * @param width Grid width.
     * @param height Grid height.
     * @param topLeft Top-left corner in normalized coordinates.
     * @param bottomRight Bottom-right corner in normalized coordinates.
     *
     * @return Generated contour set.
     */
    static ContourPointsSet generateRectangle(int width, int height, Point2D_f topLeft,
                                              Point2D_f bottomRight);

private:
    /**
     * @brief Construct a BoundaryFactory.
     *
     * @param gridWidth Width of the grid (phi domain).
     * @param gridHeight Height of the grid (phi domain).
     * @param outerBoundary Reference to the outer boundary container.
     * @param innerBoundary Reference to the inner boundary container.
     */
    BoundaryFactory(int gridWidth, int gridHeight, ContourPoints& outerBoundary,
                    ContourPoints& innerBoundary);

    /**
     * @brief Generate a rectangular boundary using integer coordinates.
     *
     * @param topLeft Top-left corner of the rectangle.
     * @param bottomRight Bottom-right corner of the rectangle.
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generateRectanglePoints(Point2D_i topLeft, Point2D_i bottomRight,
                                 BoundaryOrientation orientation = BoundaryOrientation::Normal);

    /**
     * @brief Generate a rectangular boundary using normalized coordinates.
     *
     * Coordinates are expressed in [0,1] relative to the grid size.
     *
     * @param topLeft Top-left corner (normalized).
     * @param bottomRight Bottom-right corner (normalized).
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generateRectanglePoints(Point2D_f topLeft, Point2D_f bottomRight,
                                 BoundaryOrientation orientation = BoundaryOrientation::Normal);

    /**
     * @brief Generate an ellipse boundary using integer parameters.
     *
     * @param width Ellipse width (diameter).
     * @param height Ellipse height (diameter).
     * @param center Center of the ellipse.
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generateEllipsePoints(int width, int height, Point2D_i center,
                               BoundaryOrientation orientation = BoundaryOrientation::Normal);

    /**
     * @brief Generate an ellipse boundary using normalized parameters.
     *
     * Width and height are expressed as ratios relative to the grid size.
     *
     * @param widthRatio Width ratio in [0,1].
     * @param heightRatio Height ratio in [0,1].
     * @param center Center position (normalized, default = {0,0}).
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generateEllipsePoints(float widthRatio, float heightRatio, Point2D_f center = {},
                               BoundaryOrientation orientation = BoundaryOrientation::Normal);

    /**
     * @brief Generate rectangle points using integer coordinates.
     */
    void generateRectanglePoints(int x1, int y1, int x2, int y2, ContourPoints& outerBoundary,
                                 ContourPoints& innerBoundary);

    /**
     * @brief Generate rectangle points for a single contour list.
     */
    void generateRectanglePointsForOneList(ContourPoints& boundary, int x1, int y1, int x2, int y2);

    /**
     * @brief Generate ellipse points using integer parameters.
     */
    void generateEllipsePoints(int x0, int y0, int a, int b, ContourPoints& outerBoundary,
                               ContourPoints& innerBoundary);

    /**
     * @brief Build an ellipse using a midpoint algorithm (connected boundary).
     */
    void buildEllipseMidpointConnected(int x0, int y0, int a, int b, ContourPoints& outerBoundary);

    /**
     * @brief Build the inner boundary from the outer boundary.
     */
    void buildInnerContiguous(int x0, int y0, const ContourPoints& outerBoundary,
                              ContourPoints& innerBoundary);

    /**
     * @brief Debug utility to check duplicate points in a contour.
     */
    void checkDuplicates(const ContourPoints& contour);

    /**
     * @brief Add the 4 symmetric points of an ellipse.
     */
    void add4pointsInEllipse(ContourPoints& boundary, int x, int y, int x0, int y0);

    /**
     * @brief Add a point if it is not already present (local duplicate check).
     *
     * Only checks the last few inserted points for efficiency.
     */
    void addPointUnique(ContourPoints& boundary, int x, int y);

    /**
     * @brief Check if a coordinate is inside the grid.
     */
    bool insideGrid(int x, int y) const;

    /**
     * @brief Check if a point is inside the grid.
     */
    inline bool insideGrid(const Point2D_i& p) const;

    int gridWidth_;  ///< Width of the grid.
    int gridHeight_; ///< Height of the grid.

    ContourPoints& outerBoundary_; ///< Outer boundary container.
    ContourPoints& innerBoundary_; ///< Inner boundary container.
};

inline bool BoundaryFactory::insideGrid(int x, int y) const
{
    return x >= 0 && x < gridWidth_ && y >= 0 && y < gridHeight_;
}

inline bool BoundaryFactory::insideGrid(const Point2D_i& p) const
{
    return insideGrid(p.x, p.y);
}

/**
 * @brief Add symmetric points of an ellipse while respecting grid bounds.
 */
inline void BoundaryFactory::add4pointsInEllipse(ContourPoints& boundary, int x, int y, int x0,
                                                 int y0)
{
    if (insideGrid(x0 - x, y0 - y))
        addPointUnique(boundary, x0 - x, y0 - y);

    if (insideGrid(x0 - x, y0 + y))
        addPointUnique(boundary, x0 - x, y0 + y);

    if (insideGrid(x0 + x, y0 - y))
        addPointUnique(boundary, x0 + x, y0 - y);

    if (insideGrid(x0 + x, y0 + y))
        addPointUnique(boundary, x0 + x, y0 + y);
}

/**
 * @brief Add a point to the contour if it is not already present.
 *
 * This function performs a local duplicate check on the last inserted points
 * (sliding window) to avoid redundant entries while keeping performance high.
 */
inline void BoundaryFactory::addPointUnique(ContourPoints& boundary, int x, int y)
{
    const ContourPoint point{x, y};

    constexpr size_t WINDOW = 4;
    const size_t n = boundary.size();

    const size_t begin = (n > WINDOW) ? (n - WINDOW) : 0;

    for (size_t i = begin; i < n; ++i)
    {
        if (boundary[i] == point)
            return;
    }

    boundary.push_back(point);
}

} // namespace fluvel_ip
