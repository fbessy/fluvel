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
class BoundaryBuilder
{
public:
    /**
     * @brief Construct a BoundaryBuilder.
     *
     * @param phi_width1 Width of the grid (phi domain).
     * @param phi_height1 Height of the grid (phi domain).
     * @param l_out_init1 Reference to the outer boundary container.
     * @param l_in_init1 Reference to the inner boundary container.
     */
    BoundaryBuilder(int phi_width1, int phi_height1, Contour& l_out_init1, Contour& l_in_init1);

    /**
     * @brief Generate a rectangular boundary using integer coordinates.
     *
     * @param topLeft Top-left corner of the rectangle.
     * @param bottomRight Bottom-right corner of the rectangle.
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generate_rectangle_points(Point2D_i topLeft, Point2D_i bottomRight,
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
    void generate_rectangle_points(Point2D_f topLeft, Point2D_f bottomRight,
                                   BoundaryOrientation orientation = BoundaryOrientation::Normal);

    /**
     * @brief Generate an ellipse boundary using integer parameters.
     *
     * @param width Ellipse width (diameter).
     * @param height Ellipse height (diameter).
     * @param center Center of the ellipse.
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generate_ellipse_points(int width, int height, Point2D_i center,
                                 BoundaryOrientation orientation = BoundaryOrientation::Normal);

    /**
     * @brief Generate an ellipse boundary using normalized parameters.
     *
     * Width and height are expressed as ratios relative to the grid size.
     *
     * @param width_ratio Width ratio in [0,1].
     * @param height_ratio Height ratio in [0,1].
     * @param center Center position (normalized, default = {0,0}).
     * @param orientation Boundary orientation (Normal or Reversed).
     */
    void generate_ellipse_points(float width_ratio, float height_ratio, Point2D_f center = {},
                                 BoundaryOrientation orientation = BoundaryOrientation::Normal);

private:
    /**
     * @brief Generate rectangle points using integer coordinates.
     */
    void generate_rectangle_points(int x1, int y1, int x2, int y2, Contour& list_out,
                                   Contour& list_in);

    /**
     * @brief Generate rectangle points for a single contour list.
     */
    void generate_rectangle_points_for_one_list(Contour& list_init, int x1, int y1, int x2, int y2);

    /**
     * @brief Generate ellipse points using integer parameters.
     */
    void generate_ellipse_points(int x0, int y0, int a, int b, Contour& list_out, Contour& list_in);

    /**
     * @brief Build an ellipse using a midpoint algorithm (connected boundary).
     */
    void build_ellipse_midpoint_connected(int x0, int y0, int a, int b, Contour& list_out);

    /**
     * @brief Build the inner boundary from the outer boundary.
     */
    void build_inner_contiguous(int x0, int y0, const Contour& outerBoundary,
                                Contour& innerBoundary);

    /**
     * @brief Debug utility to check duplicate points in a contour.
     */
    void check_duplicates(const Contour& contour);

    /**
     * @brief Add the 4 symmetric points of an ellipse.
     */
    void add_4_points_in_ellipse(Contour& list_init, int x, int y, int x0, int y0);

    /**
     * @brief Add a point if it is not already present (local duplicate check).
     *
     * Only checks the last few inserted points for efficiency.
     */
    void add_point_unique(Contour& out, int x, int y);

    /**
     * @brief Check if a coordinate is inside the grid.
     */
    bool inside_grid(int x, int y) const;

    /**
     * @brief Check if a point is inside the grid.
     */
    inline bool inside_grid(const Point2D_i& p) const;

    int grid_width_;  ///< Width of the grid.
    int grid_height_; ///< Height of the grid.

    Contour& Lout_init_; ///< Outer boundary container.
    Contour& Lin_init_;  ///< Inner boundary container.
};

inline bool BoundaryBuilder::inside_grid(int x, int y) const
{
    return x >= 0 && x < grid_width_ && y >= 0 && y < grid_height_;
}

inline bool BoundaryBuilder::inside_grid(const Point2D_i& p) const
{
    return inside_grid(p.x, p.y);
}

/**
 * @brief Add symmetric points of an ellipse while respecting grid bounds.
 */
inline void BoundaryBuilder::add_4_points_in_ellipse(Contour& list, int x, int y, int x0, int y0)
{
    if (inside_grid(x0 - x, y0 - y))
        add_point_unique(list, x0 - x, y0 - y);

    if (inside_grid(x0 - x, y0 + y))
        add_point_unique(list, x0 - x, y0 + y);

    if (inside_grid(x0 + x, y0 - y))
        add_point_unique(list, x0 + x, y0 - y);

    if (inside_grid(x0 + x, y0 + y))
        add_point_unique(list, x0 + x, y0 + y);
}

/**
 * @brief Add a point to the contour if it is not already present.
 *
 * This function performs a local duplicate check on the last inserted points
 * (sliding window) to avoid redundant entries while keeping performance high.
 */
inline void BoundaryBuilder::add_point_unique(Contour& out, int x, int y)
{
    const ContourPoint p{x, y};

    constexpr size_t WINDOW = 4;
    const size_t n = out.size();

    const size_t begin = (n > WINDOW) ? (n - WINDOW) : 0;

    for (size_t i = begin; i < n; ++i)
    {
        if (out[i] == p)
            return;
    }

    out.push_back(p);
}

} // namespace fluvel_ip
