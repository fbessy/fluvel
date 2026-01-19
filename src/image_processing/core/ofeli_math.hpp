#ifndef OFELI_MATH_HPP
#define OFELI_MATH_HPP

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

}

}

#endif // OFELI_MATH_HPP
