// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <cassert>
#include <cmath>
#include <compare>

#ifndef Q_MOC_RUN
#include <concepts>
#endif

#include <cstdint>
#include <type_traits>

namespace fluvel_ip
{

/**
 * @brief 32-bit BGRA pixel layout.
 *
 * Channel order in memory:
 * - blue
 * - green
 * - red
 * - alpha
 */
struct Bgra32
{
    unsigned char blue;  ///< Blue channel
    unsigned char green; ///< Green channel
    unsigned char red;   ///< Red channel
    unsigned char alpha; ///< Alpha channel
};

/**
 * @brief Concept for arithmetic types.
 *
 * Accepts both integral and floating-point types.
 */
template <typename T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;

/**
 * @brief Generic RGB color container.
 *
 * @tparam T Channel type (e.g. uint8_t, float, int)
 */
template <Arithmetic T>
    requires std::three_way_comparable<T>
struct Rgb
{
    T red{};   ///< Red channel
    T green{}; ///< Green channel
    T blue{};  ///< Blue channel

    /// Default constructor.
    constexpr Rgb() = default;

    /**
     * @brief Construct RGB color.
     */
    constexpr Rgb(T r, T g, T b)
        : red(r)
        , green(g)
        , blue(b)
    {
    }

    /**
     * @brief Conversion constructor between RGB types.
     */
    template <typename U>
    explicit Rgb(const Rgb<U>& other)
        : red(static_cast<T>(other.red))
        , green(static_cast<T>(other.green))
        , blue(static_cast<T>(other.blue))
    {
    }

    /**
     * @brief Default comparison operator (C++20).
     *      * Generates all comparison operators (==, !=, <, <=, >, >=)
     * by comparing members in declaration order.
     *      * @note Requires <compare>.
     */
    auto operator<=>(const Rgb&) const = default;

    /**
     * @brief In-place addition.
     */
    template <Arithmetic U>
    Rgb& operator+=(const Rgb<U>& rhs)
    {
        red += static_cast<T>(rhs.red);
        green += static_cast<T>(rhs.green);
        blue += static_cast<T>(rhs.blue);
        return *this;
    }

    /**
     * @brief In-place subtraction.
     *
     * @note Requires signed type if T is integral.
     */
    template <Arithmetic U>
        requires(!std::integral<T> || std::signed_integral<T>)
    Rgb& operator-=(const Rgb<U>& rhs)
    {
        red -= static_cast<T>(rhs.red);
        green -= static_cast<T>(rhs.green);
        blue -= static_cast<T>(rhs.blue);
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication.
     */
    template <Arithmetic U>
    Rgb& operator*=(const Rgb<U>& rhs)
    {
        red *= static_cast<T>(rhs.red);
        green *= static_cast<T>(rhs.green);
        blue *= static_cast<T>(rhs.blue);
        return *this;
    }

    /// Addition operator.
    template <Arithmetic U>
    Rgb operator+(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result += rhs;
        return result;
    }

    /// Subtraction operator.
    template <Arithmetic U>
    Rgb operator-(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result -= rhs;
        return result;
    }

    /// Component-wise multiplication operator.
    template <Arithmetic U>
    Rgb operator*(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result *= rhs;
        return result;
    }

    /**
     * @brief Scalar division.
     *
     * @return Floating-point RGB.
     */
    template <Arithmetic U>
    Rgb<float> operator/(U denom) const
    {
        if constexpr (std::floating_point<U>)
        {
            assert(std::abs(denom) > std::numeric_limits<U>::epsilon());
        }
        else
        {
            assert(denom != 0);
        }

        return {static_cast<float>(red) / denom, static_cast<float>(green) / denom,
                static_cast<float>(blue) / denom};
    }

    /**
     * @brief Round components to an integral RGB type.
     */
    template <std::integral I>
    Rgb<I> rounded() const
    {
        return {static_cast<I>(std::lround(red)), static_cast<I>(std::lround(green)),
                static_cast<I>(std::lround(blue))};
    }

    /**
     * @brief Sum of components.
     */
    auto scalar() const
    {
        using R = std::conditional_t<std::integral<T>, int, T>;
        return static_cast<R>(red) + static_cast<R>(green) + static_cast<R>(blue);
    }

    /// Scalar multiplication.
    template <Arithmetic U>
    Rgb operator*(U scalar) const
    {
        return {static_cast<T>(red * scalar), static_cast<T>(green * scalar),
                static_cast<T>(blue * scalar)};
    }

    /// In-place scalar multiplication.
    template <Arithmetic U>
    Rgb& operator*=(U scalar)
    {
        red = static_cast<T>(red * scalar);
        green = static_cast<T>(green * scalar);
        blue = static_cast<T>(blue * scalar);
        return *this;
    }
};

/// 8-bit RGB
using Rgb_uc = Rgb<unsigned char>;
/// 64-bit integer RGB (for accumulation)
using Rgb_64i = Rgb<int64_t>;
/// Floating-point RGB
using Rgb_f = Rgb<float>;

/**
 * @brief CIE L*a*b* color representation.
 */
struct Lab_f
{
    float L; ///< Lightness
    float a; ///< Green-red axis
    float b; ///< Blue-yellow axis
};

/**
 * @brief CIE L*u*v* color representation.
 */
struct Luv_f
{
    float L;
    float u;
    float v;
};

/**
 * @brief CIE XYZ color representation.
 */
struct Xyz_f
{
    float X;
    float Y;
    float Z;
};

/**
 * @brief Generic 3-component integer container.
 *
 * Used for color representations (e.g. YUV) or generic vector operations.
 */
struct Components_3i
{
    int c1; ///< First component
    int c2; ///< Second component
    int c3; ///< Third component

    /**
     * @brief Default comparison operator (C++20).
     *      * Generates all comparison operators (==, !=, <, <=, >, >=)
     * by comparing members in declaration order.
     *      * @note Requires <compare>.
     */
    auto operator<=>(const Components_3i&) const = default;

    Components_3i& operator+=(const Components_3i& rhs)
    {
        c1 += rhs.c1;
        c2 += rhs.c2;
        c3 += rhs.c3;
        return *this;
    }

    Components_3i& operator-=(const Components_3i& rhs)
    {
        c1 -= rhs.c1;
        c2 -= rhs.c2;
        c3 -= rhs.c3;
        return *this;
    }

    Components_3i& operator*=(const Components_3i& rhs)
    {
        c1 *= rhs.c1;
        c2 *= rhs.c2;
        c3 *= rhs.c3;
        return *this;
    }

    /// Sum of components.
    int scalar() const
    {
        return c1 + c2 + c3;
    }

    friend Components_3i operator+(Components_3i lhs, const Components_3i& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Components_3i operator-(Components_3i lhs, const Components_3i& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Components_3i operator*(Components_3i lhs, const Components_3i& rhs)
    {
        lhs *= rhs;
        return lhs;
    }
};

/// Alias for 3-component color representation
using Color_3i = Components_3i;

/**
 * @brief Scale normalized values to [0,255] and round.
 */
inline Color_3i scaleAndRound(float c1, float c2, float c3)
{
    constexpr float scale = 255.f;

    return {static_cast<int>(std::lround(scale * c1)), static_cast<int>(std::lround(scale * c2)),
            static_cast<int>(std::lround(scale * c3))};
}

namespace color
{

/**
 * @brief Inverse sRGB gamma correction (R' -> R).
 */
constexpr float INV_GAMMA_CORRECTION(float val)
{
    return val <= 0.0404482362771076f ? val / 12.92f : std::pow(((val) + 0.055f) / 1.055f, 2.4f);
}

/** @brief XYZ color of the D65 white point */
constexpr float kWhitePointX = 0.950456f;
constexpr float kWhitePointY = 1.f;
constexpr float kWhitePointZ = 1.088754f;

/** @brief *u*v color of the D65 white point */
constexpr float kWhitePointDenom = kWhitePointX + 15.f * kWhitePointY + 3.f * kWhitePointZ;
constexpr float kWhitePointU = 4.f * kWhitePointX / kWhitePointDenom;
constexpr float kWhitePointV = 9.f * kWhitePointY / kWhitePointDenom;

constexpr float kLabEpsilon = 216.f / 24389.f;
constexpr float kLabKappa = 841.f / 108.f;
constexpr float kLabDelta = 4.f / 29.f;

/**
 * @brief CIE L*a*b* helper function f(t).
 */
inline float LAB_FUNC(float val)
{
    return val >= kLabEpsilon ? std::cbrtf(val) : kLabKappa * (val) + kLabDelta;
}

/**
 * @brief Convert RGB to XYZ color space.
 */
inline Xyz_f rgb_to_xyz(const Rgb_uc& rgb)
{
    Rgb_f rgb_f = rgb / 255.f;

    rgb_f.red = INV_GAMMA_CORRECTION(rgb_f.red);
    rgb_f.green = INV_GAMMA_CORRECTION(rgb_f.green);
    rgb_f.blue = INV_GAMMA_CORRECTION(rgb_f.blue);

    return {0.4123956f * rgb_f.red + 0.35758343f * rgb_f.green + 0.18049265f * rgb_f.blue,
            0.21258623f * rgb_f.red + 0.7151703f * rgb_f.green + 0.0722005f * rgb_f.blue,
            0.019297216f * rgb_f.red + 0.11918387f * rgb_f.green + 0.95049715f * rgb_f.blue};
}

/**
 * @brief Convert XYZ to Lab color space.
 */
inline Lab_f xyz_to_Lab(const Xyz_f& xyz)
{
    Xyz_f tmp = xyz;

    tmp.X /= kWhitePointX;
    tmp.Y /= kWhitePointY;
    tmp.Z /= kWhitePointZ;
    tmp.X = LAB_FUNC(tmp.X);
    tmp.Y = LAB_FUNC(tmp.Y);
    tmp.Z = LAB_FUNC(tmp.Z);

    return {116.f * tmp.Y - 16.f, 500.f * (tmp.X - tmp.Y), 200.f * (tmp.Y - tmp.Z)};
}

/**
 * @brief Convert XYZ to Luv color space.
 */
inline Luv_f xyz_to_Luv(const Xyz_f& xyz)
{
    Xyz_f tmp = xyz;
    float u1, v1, denom;

    denom = tmp.X + 15.f * tmp.Y + 3.f * tmp.Z;

    if (denom > 0.f)
    {
        u1 = (4.f * tmp.X) / denom;
        v1 = (9.f * tmp.Y) / denom;
    }
    else
    {
        u1 = 0.f;
        v1 = 0.f;
    }

    tmp.Y /= kWhitePointY;
    tmp.Y = LAB_FUNC(tmp.Y);

    float L = 116.f * tmp.Y - 16.f;

    return {L, 13.f * L * (u1 - kWhitePointU), 13.f * L * (v1 - kWhitePointV)};
}

/**
 * @brief Convert RGB to Lab color space.
 */
inline Lab_f rgb_to_Lab(const Rgb_uc& rgb)
{
    return xyz_to_Lab(rgb_to_xyz(rgb));
}

/**
 * @brief Convert RGB to Luv color space.
 */
inline Luv_f rgb_to_Luv(const Rgb_uc& rgb)
{
    return xyz_to_Luv(rgb_to_xyz(rgb));
}

/**
 * @brief Convert RGB to YUV (integer approximation).
 */
inline Color_3i rgb_to_yuv(const Rgb_uc& rgb)
{
    return {((66 * int(rgb.red) + 129 * int(rgb.green) + 25 * int(rgb.blue) + 128) >> 8) + 16,
            ((-38 * int(rgb.red) - 74 * int(rgb.green) + 112 * int(rgb.blue) + 128) >> 8) + 128,
            ((112 * int(rgb.red) - 94 * int(rgb.green) - 18 * int(rgb.blue) + 128) >> 8) + 128};
}

} // namespace color

} // namespace fluvel_ip
