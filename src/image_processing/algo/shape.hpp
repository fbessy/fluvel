// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point.hpp"
#include <vector>

namespace fluvel_ip
{

/**
 * @brief Geometric shape represented as a set of 2D integer points.
 *
 * This class is primarily used to represent contour shapes and supports
 * operations such as centroid computation and Hausdorff distance evaluation.
 */
class Shape
{
public:
    /**
     * @brief Default constructor.
     */
    Shape();

    /**
     * @brief Reserve memory for points.
     *
     * @param elem_alloc_size Number of elements to reserve.
     */
    void reserve(size_t elem_alloc_size);

    /**
     * @brief Remove all points from the shape.
     */
    void clear();

    /**
     * @brief Add a point to the shape.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     */
    void pushBack(int x, int y);

    /**
     * @brief Add a point to the shape.
     *
     * @param p Point to add.
     */
    void pushBack(const Point2D_i& p);

    /**
     * @brief Add a point to the shape (move version).
     *
     * @param p Point to move into the container.
     */
    void pushBack(Point2D_i&& p);

    /**
     * @brief Swap this shape with another in constant time.
     *
     * @param other Shape to swap with.
     */
    void swap(Shape& other) noexcept;

    /**
     * @brief Randomly shuffle the points of the shape.
     *
     * Useful for stochastic algorithms or randomized sampling.
     */
    void shufflePoints();

    /**
     * @brief Compute the centroid of the shape.
     *
     * The result is stored internally and accessible via centroid().
     */
    void calculateCentroid();

    /**
     * @brief Check if the shape is valid.
     *
     * A valid shape typically contains enough points for geometric
     * computations such as Hausdorff distance.
     *
     * @return True if the shape is valid.
     */
    bool isValid() const;

    /**
     * @brief Get the points composing the shape.
     *
     * @return Const reference to the internal vector of points.
     */
    const std::vector<Point2D_i>& points() const
    {
        return points_;
    }

    /**
     * @brief Get the centroid of the shape.
     *
     * @return Centroid as a floating-point 2D point.
     */
    const Point2D_f& centroid() const
    {
        return centroid_;
    }

    /**
     * @brief Compute the diagonal length of a grid.
     *
     * Useful for normalization (e.g. Hausdorff distance).
     *
     * @param gridWidth Grid width.
     * @param gridHeight Grid height.
     * @return Diagonal length.
     */
    static float gridDiagonal(int gridWidth, int gridHeight);

private:
    /// List of points composing the shape.
    std::vector<Point2D_i> points_;

    /// Centroid of the shape.
    Point2D_f centroid_;
};

} // namespace fluvel_ip
