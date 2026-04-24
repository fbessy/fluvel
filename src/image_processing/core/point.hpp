// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_ip
{

/**
 * @brief Generic 2D point structure.
 *
 * Represents a 2D coordinate or vector with components (x, y).
 * Can be used for:
 * - pixel coordinates
 * - geometric points
 * - offsets or vectors
 *
 * @tparam T Coordinate type (e.g., int, float).
 */
template <typename T>
struct Point2D
{
    T x; ///< X coordinate
    T y; ///< Y coordinate

    /// Default constructor.
    Point2D() = default;

    /**
     * @brief Construct a point from coordinates.
     *
     * @param x_ X coordinate.
     * @param y_ Y coordinate.
     */
    constexpr Point2D(T x_, T y_) noexcept
        : x(x_)
        , y(y_)
    {
    }

    /**
     * @brief Equality comparison.
     */
    bool operator==(const Point2D& other) const noexcept
    {
        return x == other.x && y == other.y;
    }

    /**
     * @brief Inequality comparison.
     */
    bool operator!=(const Point2D& other) const noexcept
    {
        return !(*this == other);
    }

    /**
     * @brief In-place addition.
     */
    Point2D& operator+=(const Point2D& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    /**
     * @brief Addition operator.
     */
    friend Point2D operator+(Point2D lhs, const Point2D& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }
};

/**
 * @brief Integer 2D point.
 *
 * Commonly used for pixel coordinates and grid indexing.
 */
using Point2D_i = Point2D<int>;

/**
 * @brief Floating-point 2D point.
 *
 * Used for geometric computations and continuous coordinates.
 */
using Point2D_f = Point2D<float>;

} // namespace fluvel_ip
