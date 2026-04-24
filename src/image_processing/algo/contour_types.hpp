// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "grid2d.hpp"

#include <cassert>
#include <string_view>
#include <vector>

namespace fluvel_ip
{

/**
 * @brief Discrete values of the level-set function.
 *
 * The level-set is represented using 4 discrete states:
 * - InsideRegion: strictly inside the contour
 * - InteriorBoundary: inner boundary (Lin)
 * - ExteriorBoundary: outer boundary (Lout)
 * - OutsideRegion: strictly outside the contour
 *
 * @note The zero level-set is conceptual and not explicitly stored.
 */
enum class PhiValue : int8_t
{
    InsideRegion = -3,     ///< Inside region
    InteriorBoundary = -1, ///< Inner boundary (Lin)
    ExteriorBoundary = 1,  ///< Outer boundary (Lout)
    OutsideRegion = 3      ///< Outside region
};

/**
 * @brief Discrete level-set representation.
 */
using DiscreteLevelSet = Grid2D<PhiValue>;

namespace phi_value
{

/**
 * @brief Get the sign of a PhiValue.
 *
 * @param v Phi value.
 * @return -1 for inside, +1 for outside.
 */
constexpr int phiSign(PhiValue v);

/**
 * @brief Check if a value is inside the contour.
 */
constexpr bool isInside(PhiValue v);

/**
 * @brief Check if a value is outside the contour.
 */
constexpr bool isOutside(PhiValue v);

/**
 * @brief Check if two values are on opposite sides of the contour.
 */
constexpr bool differentSide(PhiValue a, PhiValue b);

/**
 * @brief Discrete sign of level-set function.
 *
 * Returns -1 for interior side, +1 for exterior side.
 */
constexpr int phiSign(PhiValue v)
{
    switch (v)
    {
        case PhiValue::InsideRegion:
        case PhiValue::InteriorBoundary:
            return -1;

        case PhiValue::ExteriorBoundary:
        case PhiValue::OutsideRegion:
            return 1;
    }

    assert(false);
    return 0; // unreachable
}

constexpr bool isInside(PhiValue v)
{
    return phiSign(v) < 0;
}

constexpr bool isOutside(PhiValue v)
{
    return !isInside(v);
}

constexpr bool differentSide(PhiValue a, PhiValue b)
{
    return phiSign(a) * phiSign(b) < 0;
}

} // namespace phi_value

/**
 * @brief Discrete speed direction for contour evolution.
 *
 * Represents the direction of motion of a contour point.
 */
enum class SpeedValue : int8_t
{
    GoInward = -1, ///< Move toward inside
    NoMove = 0,    ///< No movement
    GoOutward = 1  ///< Move toward outside
};

namespace speed_value
{

/**
 * @brief Convert an integer speed to a discrete speed value.
 *
 * @param speed Signed speed value.
 * @return Corresponding discrete speed.
 */
constexpr SpeedValue get_discrete_speed(int speed);

constexpr SpeedValue get_discrete_speed(int speed)
{
    if (speed < 0)
        return SpeedValue::GoInward;
    if (speed > 0)
        return SpeedValue::GoOutward;
    return SpeedValue::NoMove;
}

} // namespace speed_value

/**
 * @brief Represents a point belonging to a contour.
 *
 * Stores:
 * - spatial coordinates,
 * - current evolution speed (direction).
 */
class ContourPoint
{
public:
    /**
     * @brief Construct a contour point from coordinates.
     */
    ContourPoint(int x, int y)
        : x_(x)
        , y_(y)
        , speed_(SpeedValue::NoMove)
    {
    }

    /**
     * @brief Construct a contour point from a 2D point.
     */
    ContourPoint(const Point2D_i& p)
        : x_(p.x)
        , y_(p.y)
        , speed_(SpeedValue::NoMove)
    {
    }

    /**
     * @brief Get position as a 2D point.
     */
    Point2D_i pos() const noexcept
    {
        return {x_, y_};
    }

    /**
     * @brief Get x coordinate.
     */
    int x() const noexcept
    {
        return x_;
    }

    /**
     * @brief Get y coordinate.
     */
    int y() const noexcept
    {
        return y_;
    }

    /**
     * @brief Get current speed value.
     */
    SpeedValue speed() const noexcept
    {
        return speed_;
    }

    /**
     * @brief Set the speed value.
     */
    void setSpeed(SpeedValue s) noexcept
    {
        speed_ = s;
    }

    /**
     * @brief Equality comparison.
     */
    bool operator==(const ContourPoint& other) const noexcept
    {
        return x_ == other.x_ && y_ == other.y_;
    }

    /**
     * @brief Inequality comparison.
     */
    bool operator!=(const ContourPoint& other) const noexcept
    {
        return !(*this == other);
    }

private:
    int x_; ///< X coordinate
    int y_; ///< Y coordinate

    /**
     * @brief Pending speed used by the algorithm to drive contour evolution.
     */
    SpeedValue speed_;
};

/**
 * @brief Container for contour points.
 */
using Contour = std::vector<ContourPoint>;

/**
 * @brief Exported geometric contour (no speed information).
 */
using ExportedContour = std::vector<Point2D_i>;

/**
 * @brief Convert a ContourPoint to a 2D point.
 */
constexpr inline Point2D_i from_ContourPoint(const ContourPoint& point)
{
    return {point.x(), point.y()};
}

/**
 * @brief Pixel connectivity used by the algorithm.
 *
 * Determines neighborhood definition:
 * - Four: 4-connected grid
 * - Eight: 8-connected grid
 */
enum class Connectivity
{
    Four,
    Eight
};

/**
 * @brief Convert connectivity to string.
 */
inline constexpr const char* to_string(Connectivity connectivity)
{
    switch (connectivity)
    {
        case Connectivity::Four:
            return "4";
        case Connectivity::Eight:
            return "8";
    }

    return "4";
}

/**
 * @brief Parse connectivity from string.
 */
inline Connectivity connectivity_from_string(std::string_view c)
{
    if (c == "4")
        return Connectivity::Four;
    if (c == "8")
        return Connectivity::Eight;

    return Connectivity::Four;
}

} // namespace fluvel_ip
