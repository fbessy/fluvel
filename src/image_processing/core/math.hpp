#ifndef MATH_HPP
#define MATH_HPP

namespace ofeli_ip::math
{

template<typename T>
constexpr T square(T v) noexcept
{
    return v * v;
}

}

#endif // MATH_HPP
