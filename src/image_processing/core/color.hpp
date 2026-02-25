// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace ofeli_ip
{

struct Bgra32
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
};

template <typename T>
concept Arithmetic = std::integral<T> || std::floating_point<T>;

template <Arithmetic T> struct Rgb
{
    T red{};
    T green{};
    T blue{};

    constexpr Rgb() = default;

    constexpr Rgb(T r, T g, T b)
        : red(r)
        , green(g)
        , blue(b)
    {
    }

    template <typename U>
    explicit Rgb(const Rgb<U>& other)
        : red(static_cast<T>(other.red))
        , green(static_cast<T>(other.green))
        , blue(static_cast<T>(other.blue))
    {
    }

    bool operator==(const Rgb& other) const noexcept
    {
        return red == other.red && green == other.green && blue == other.blue;
    }

    bool operator!=(const Rgb& other) const noexcept
    {
        return !(*this == other);
    }

    template <Arithmetic U> Rgb& operator+=(const Rgb<U>& rhs)
    {
        red += static_cast<T>(rhs.red);
        green += static_cast<T>(rhs.green);
        blue += static_cast<T>(rhs.blue);
        return *this;
    }

    template <typename U> Rgb& operator-=(const Rgb<U>& rhs)
    {
        static_assert(!std::integral<T> || std::signed_integral<T>,
                      "Rgb<T>::operator-= requires T to be signed if integral");

        red -= static_cast<T>(rhs.red);
        green -= static_cast<T>(rhs.green);
        blue -= static_cast<T>(rhs.blue);
        return *this;
    }

    template <Arithmetic U> Rgb& operator*=(const Rgb<U>& rhs)
    {
        red *= static_cast<T>(rhs.red);
        green *= static_cast<T>(rhs.green);
        blue *= static_cast<T>(rhs.blue);
        return *this;
    }

    // ---- opérateurs non-mutants ----
    template <Arithmetic U> Rgb operator+(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result += rhs;
        return result;
    }

    template <Arithmetic U> Rgb operator-(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result -= rhs;
        return result;
    }

    template <Arithmetic U> Rgb operator*(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result *= rhs;
        return result;
    }

    template <Arithmetic U> Rgb<float> operator/(U denom) const
    {
        return {static_cast<float>(red) / denom, static_cast<float>(green) / denom,
                static_cast<float>(blue) / denom};
    }

    template <std::integral I> Rgb<I> rounded() const
    {
        return {static_cast<I>(std::lround(red)), static_cast<I>(std::lround(green)),
                static_cast<I>(std::lround(blue))};
    }

    auto scalar() const
    {
        using R = std::conditional_t<std::integral<T>, int, T>;
        return static_cast<R>(red) + static_cast<R>(green) + static_cast<R>(blue);
    }

    template <Arithmetic U> Rgb operator*(U scalar) const
    {
        return {static_cast<T>(red * scalar), static_cast<T>(green * scalar),
                static_cast<T>(blue * scalar)};
    }

    template <Arithmetic U> Rgb& operator*=(U scalar)
    {
        red = static_cast<T>(red * scalar);
        green = static_cast<T>(green * scalar);
        blue = static_cast<T>(blue * scalar);
        return *this;
    }
};

using Rgb_uc = Rgb<unsigned char>;
using Rgb_64i = Rgb<int64_t>;
using Rgb_f = Rgb<float>;

struct Lab_f
{
    float L;
    float a;
    float b;
};

struct Luv_f
{
    float L;
    float u;
    float v;
};

struct Xyz_f
{
    float X;
    float Y;
    float Z;
};

//! Generic 3 components.
struct Components_3i
{
    int c1;
    int c2;
    int c3;

    bool operator==(const Components_3i&) const = default;

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

using Color_3i = Components_3i;

inline Color_3i scale_and_round(float c1, float c2, float c3)
{
    constexpr float scale = 255.f;

    return {static_cast<int>(std::lround(scale * c1)), static_cast<int>(std::lround(scale * c2)),
            static_cast<int>(std::lround(scale * c3))};
}

namespace color
{

//! Color space conversion functions.
//!
static Xyz_f rgb_to_xyz(const Rgb_uc& rgb);
static Lab_f xyz_to_Lab(const Xyz_f& xyz);
static Luv_f xyz_to_Luv(const Xyz_f& xyz);
static Lab_f rgb_to_Lab(const Rgb_uc& rgb);
static Luv_f rgb_to_Luv(const Rgb_uc& rgb);

/**
 * @brief Inverse sRGB gamma correction, transforms R' to R
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
 * @brief CIE L*a*b* f function (used to convert XYZ to L*a*b*)
 * http://en.wikipedia.org/wiki/Lab_color_space
 */
inline float LAB_FUNC(float val)
{
    return val >= kLabEpsilon ? std::cbrtf(val) : kLabKappa * (val) + kLabDelta;
}

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
 * Convert CIE XYZ to CIE L*u*v* (CIELUV) with the D65 white point
 *
 * @param L, u, v pointers to hold the result
 * @param X, Y, Z the input XYZ values
 *
 * Wikipedia: http://en.wikipedia.org/wiki/CIELUV_color_space
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

inline Lab_f rgb_to_Lab(const Rgb_uc& rgb)
{
    return xyz_to_Lab(rgb_to_xyz(rgb));
}

inline Luv_f rgb_to_Luv(const Rgb_uc& rgb)
{
    return xyz_to_Luv(rgb_to_xyz(rgb));
}

inline Color_3i rgb_to_yuv(const Rgb_uc& rgb)
{
    return {((66 * int(rgb.red) + 129 * int(rgb.green) + 25 * int(rgb.blue) + 128) >> 8) + 16,
            ((-38 * int(rgb.red) - 74 * int(rgb.green) + 112 * int(rgb.blue) + 128) >> 8) + 128,
            ((112 * int(rgb.red) - 94 * int(rgb.green) - 18 * int(rgb.blue) + 128) >> 8) + 128};
}

} // namespace color

} // namespace ofeli_ip
