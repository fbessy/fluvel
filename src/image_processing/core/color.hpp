#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>
#include <concepts>
#include <type_traits>
#include <cmath>

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

template <Arithmetic T>
struct Rgb
{
    T red{};
    T green{};
    T blue{};

    constexpr Rgb() = default;

    constexpr Rgb(T r, T g, T b)
        : red(r), green(g), blue(b) {}

    template <typename U>
    explicit Rgb(const Rgb<U>& other)
        : red(static_cast<T>(other.red)),
          green(static_cast<T>(other.green)),
          blue(static_cast<T>(other.blue))
    {}

    template <Arithmetic U>
    Rgb& operator+=(const Rgb<U>& rhs)
    {
        red   += static_cast<T>(rhs.red);
        green += static_cast<T>(rhs.green);
        blue  += static_cast<T>(rhs.blue);
        return *this;
    }

    template <typename U>
    Rgb& operator-=(const Rgb<U>& rhs)
    {
        static_assert(
            !std::integral<T> || std::signed_integral<T>,
            "Rgb<T>::operator-= requires T to be signed if integral"
            );

        red   -= static_cast<T>(rhs.red);
        green -= static_cast<T>(rhs.green);
        blue  -= static_cast<T>(rhs.blue);
        return *this;
    }

    template <Arithmetic U>
    Rgb& operator*=(const Rgb<U>& rhs)
    {
        red   *= static_cast<T>(rhs.red);
        green *= static_cast<T>(rhs.green);
        blue  *= static_cast<T>(rhs.blue);
        return *this;
    }

    // ---- opérateurs non-mutants ----
    template <Arithmetic U>
    Rgb operator+(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result += rhs;
        return result;
    }

    template <Arithmetic U>
    Rgb operator-(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result -= rhs;
        return result;
    }

    template <Arithmetic U>
    Rgb operator*(const Rgb<U>& rhs) const
    {
        Rgb result = *this;
        result *= rhs;
        return result;
    }

    template <Arithmetic U>
    Rgb<float> operator/(U denom) const
    {
        return {
            static_cast<float>(red)   / denom,
            static_cast<float>(green) / denom,
            static_cast<float>(blue)  / denom
        };
    }

    template <std::integral I>
    Rgb<I> rounded() const
    {
        return {
            static_cast<I>(std::llround(red)),
            static_cast<I>(std::llround(green)),
            static_cast<I>(std::llround(blue))
        };
    }

    T scalar() const
    {
        return red+green+blue;
    }
};



using Rgb_uc   = Rgb<unsigned char>;
using Rgb_64i  = Rgb<int64_t>;
using Rgb_f    = Rgb<float>;

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

    friend Components_3i operator+(Components_3i lhs,
                                   const Components_3i& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Components_3i operator-(Components_3i lhs,
                                   const Components_3i& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Components_3i operator*(Components_3i lhs,
                                   const Components_3i& rhs)
    {
        lhs *= rhs;
        return lhs;
    }
};

using Color_3i = Components_3i;

namespace color
{

//! Color space conversion functions.
//!
static Xyz_f rgb_to_xyz(const Rgb_uc& rgb);
static Lab_f xyz_to_Lab(const Xyz_f&  xyz);
static Luv_f xyz_to_Luv(const Xyz_f&  xyz);
static Lab_f rgb_to_Lab(const Rgb_uc& rgb);
static Luv_f rgb_to_Luv(const Rgb_uc& rgb);

/**
 * @brief Inverse sRGB gamma correction, transforms R' to R
 */
constexpr float INV_GAMMA_CORRECTION(float val)
{
    return val <= 0.0404482362771076f ?
               val/12.92f : std::pow(((val) + 0.055f)/1.055f, 2.4f);
}

/** @brief XYZ color of the D65 white point */
constexpr auto WHITE_POINT_X = 0.950456f;
constexpr auto WHITE_POINT_Y = 1.f;
constexpr auto WHITE_POINT_Z = 1.088754f;

/** @brief *u*v color of the D65 white point */
constexpr auto WHITE_POINT_DENOM = WHITE_POINT_X + 15.f*WHITE_POINT_Y + 3.f*WHITE_POINT_Z;
constexpr auto WHITE_POINT_U     = 4.f*WHITE_POINT_X / WHITE_POINT_DENOM;
constexpr auto WHITE_POINT_V     = 9.f*WHITE_POINT_Y / WHITE_POINT_DENOM;

/**
 * @brief CIE L*a*b* f function (used to convert XYZ to L*a*b*)
 * http://en.wikipedia.org/wiki/Lab_color_space
 */
constexpr float LAB_FUNC(float val)
{
    return val >= float(8.85645167903563082e-3) ?
               std::cbrtf(val) : (841.f/108.f)*(val) + (4.f/29.f);
}

inline Xyz_f rgb_to_xyz(const Rgb_uc& rgb)
{
    Rgb_f rgb_f = rgb / 255.f;

    rgb_f.red   = INV_GAMMA_CORRECTION(rgb_f.red);
    rgb_f.green = INV_GAMMA_CORRECTION(rgb_f.green);
    rgb_f.blue  = INV_GAMMA_CORRECTION(rgb_f.blue);

    return { (float)(0.4123955889674142161*rgb_f.red  + 0.3575834307637148171*rgb_f.green + 0.1804926473817015735*rgb_f.blue),
             (float)(0.2125862307855955516*rgb_f.red  + 0.7151703037034108499*rgb_f.green + 0.07220049864333622685*rgb_f.blue),
             (float)(0.01929721549174694484*rgb_f.red + 0.1191838645808485318*rgb_f.green + 0.9504971251315797660*rgb_f.blue) };
}

inline Lab_f xyz_to_Lab(const Xyz_f& xyz)
{
    Xyz_f tmp = xyz;

    tmp.X /= WHITE_POINT_X;
    tmp.Y /= WHITE_POINT_Y;
    tmp.Z /= WHITE_POINT_Z;
    tmp.X = LAB_FUNC(tmp.X);
    tmp.Y = LAB_FUNC(tmp.Y);
    tmp.Z = LAB_FUNC(tmp.Z);

    return { 116.f*tmp.Y - 16.f,
             500.f*(tmp.X - tmp.Y),
             200.f*(tmp.Y - tmp.Z) };
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

    denom = tmp.X + 15.f*tmp.Y + 3.f*tmp.Z;

    if( denom > 0.f )
    {
        u1 = (4.f*tmp.X) / denom;
        v1 = (9.f*tmp.Y) / denom;
    }
    else
    {
        u1 = 0.f;
        v1 = 0.f;
    }

    tmp.Y /= WHITE_POINT_Y;
    tmp.Y = LAB_FUNC(tmp.Y);

    float L = 116.f*tmp.Y - 16.f;

    return { L,
             13.f*L*(u1 - WHITE_POINT_U),
             13.f*L*(v1 - WHITE_POINT_V) };
}

inline Lab_f rgb_to_Lab(const Rgb_uc& rgb)
{
    return xyz_to_Lab( rgb_to_xyz(rgb) );
}

inline Luv_f rgb_to_Luv(const Rgb_uc& rgb)
{
    return xyz_to_Luv( rgb_to_xyz(rgb) );
}

inline Color_3i rgb_to_yuv(const Rgb_uc& rgb)
{
    return { ( (  66 * int(rgb.red) + 129 * int(rgb.green) +  25 * int(rgb.blue) + 128) >> 8) +  16,
             ( ( -38 * int(rgb.red) -  74 * int(rgb.green) + 112 * int(rgb.blue) + 128) >> 8) + 128,
             ( ( 112 * int(rgb.red) -  94 * int(rgb.green) -  18 * int(rgb.blue) + 128) >> 8) + 128 };
}

}

}

#endif // COLOR_HPP
