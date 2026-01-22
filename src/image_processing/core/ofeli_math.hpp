#ifndef OFELI_MATH_HPP
#define OFELI_MATH_HPP

#include <cmath>

namespace ofeli_ip
{

namespace math
{

template<typename T>
constexpr T square(T v) noexcept
{
    return v * v;
}

constexpr int sign(int v) noexcept
{
    if ( v > 0 )
        return  1;

    if ( v < 0 )
        return -1;

    return 0;
}

template<typename P>
inline float euclidean_distance(const P& a, const P& b)
{
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;

    // sqrt(0.f) is well-defined and returns 0.f (IEEE-754),
    // no special-case needed for coincident points.
    return std::sqrt(dx*dx + dy*dy);
}

}

}

#endif // OFELI_MATH_HPP
