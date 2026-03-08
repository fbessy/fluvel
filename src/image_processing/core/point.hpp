// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_ip
{

template <typename T> struct Point2D
{
    T x;
    T y;

    Point2D() = default;
    constexpr Point2D(T x_, T y_) noexcept
        : x(x_)
        , y(y_)
    {
    }

    bool operator==(const Point2D& other) const noexcept
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point2D& other) const noexcept
    {
        return !(*this == other);
    }

    Point2D& operator+=(const Point2D& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    friend Point2D operator+(Point2D lhs, const Point2D& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }
};

using Point2D_i = Point2D<int>;
using Point2D_f = Point2D<float>;

} // namespace fluvel_ip
