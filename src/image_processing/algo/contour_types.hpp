// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "grid2d.hpp"

#include <cassert>
#include <string_view>
#include <vector>

namespace fluvel_ip
{

enum class PhiValue : int8_t
{
    InsideRegion = -3,
    InteriorBoundary = -1,
    ExteriorBoundary = 1,
    OutsideRegion = 3
};

using DiscreteLevelSet = Grid2D<PhiValue>;

namespace phi_value
{

constexpr int phiSign(PhiValue v);
constexpr bool isInside(PhiValue v);
constexpr bool isOutside(PhiValue v);
constexpr bool differentSide(PhiValue a, PhiValue b);

/// Discrete sign of level-set function.
/// Returns -1 for interior side, +1 for exterior side.
/// Note: zero level-set is conceptual and never stored.
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

enum class SpeedValue : int8_t
{
    GoInward = -1,
    NoMove = 0,
    GoOutward = 1
};

namespace speed_value
{

//! Gets a discrete speed.
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

class ContourPoint
{
public:
    ContourPoint(int x, int y)
        : x_(x)
        , y_(y)
        , speed_(SpeedValue::NoMove)
    {
    }
    ContourPoint(const Point2D_i& p)
        : x_(p.x)
        , y_(p.y)
        , speed_(SpeedValue::NoMove)
    {
    }

    Point2D_i pos() const noexcept
    {
        return {x_, y_};
    }
    int x() const noexcept
    {
        return x_;
    }
    int y() const noexcept
    {
        return y_;
    }

    SpeedValue speed() const noexcept
    {
        return speed_;
    }

    void setSpeed(SpeedValue s) noexcept
    {
        speed_ = s;
    }

    bool operator==(const ContourPoint& other) const noexcept
    {
        return x_ == other.x_ && y_ == other.y_;
    }

    bool operator!=(const ContourPoint& other) const noexcept
    {
        return !(*this == other);
    }

private:
    int x_;
    int y_;

    //! Pending sign speed of the algorithm to drive the contour
    SpeedValue speed_;
};

using Contour = std::vector<ContourPoint>;
using ExportedContour = std::vector<Point2D_i>;

constexpr inline Point2D_i from_ContourPoint(const ContourPoint& point)
{
    return {point.x(), point.y()};
}

//! Pixel connectivity of the neighborhood used by the algorithm (4- or 8-connected).
enum class Connectivity
{
    Four,
    Eight
};

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

inline Connectivity connectivity_from_string(std::string_view c)
{
    if (c == "4")
        return Connectivity::Four;
    if (c == "8")
        return Connectivity::Eight;

    return Connectivity::Four;
}

} // namespace fluvel_ip
