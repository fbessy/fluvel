// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <cmath>

namespace fluvel_ip
{

namespace math
{

/**
 * @brief Compute the square of a value.
 *
 * @tparam T Arithmetic type.
 * @param v Input value.
 * @return v * v.
 */
template <typename T>
constexpr T square(T v) noexcept
{
    return v * v;
}

/**
 * @brief Compute the sign of an integer.
 *
 * @param v Input value.
 * @return +1 if v > 0, -1 if v < 0, 0 if v == 0.
 */
constexpr int sign(int v) noexcept
{
    if (v > 0)
        return 1;

    if (v < 0)
        return -1;

    return 0;
}

/**
 * @brief Compute the Euclidean distance between two 2D points.
 *
 * The point type P must expose members `x` and `y`.
 *
 * @tparam P Point type.
 * @param a First point.
 * @param b Second point.
 * @return Euclidean distance between a and b.
 *
 * @note sqrt(0.f) is well-defined and returns 0.f (IEEE-754),
 *       so no special handling is required for coincident points.
 */
template <typename P>
inline float euclideanDistance(const P& a, const P& b)
{
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;

    return std::sqrt(dx * dx + dy * dy);
}

} // namespace math

} // namespace fluvel_ip
