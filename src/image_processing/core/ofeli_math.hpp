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

}

}

#endif // OFELI_MATH_H
