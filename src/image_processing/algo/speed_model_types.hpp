// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"

#include <string_view>

namespace fluvel_ip
{

//! \class RegionParams
//! Specific configuration for region based active contour
struct RegionParams
{
    static constexpr int kDefaultLambdaIn = 1;
    static constexpr int kDefaultLambdaOut = 1;

    //! Weight of the inside homogeneity criterion in the Chan-Vese model
    //! (called lambda 1 in the article "Active contour without edges.").
    int lambdaIn;

    //! Weight of the outside homogeneity criterion in the Chan-Vese model
    //! (called lambda 2 in the article "Active contour without edges.").
    int lambdaOut;

    //! Default constructor.
    RegionParams()
        : lambdaIn(kDefaultLambdaIn)
        , lambdaOut(kDefaultLambdaOut)
    {
    }

    //! Destructor.
    //! Non-virtual on purpose: this is a value-type configuration struct,
    //! not intended for polymorphic use or deletion through a base pointer.
    ~RegionParams() = default;

    //! Copy constructor.
    RegionParams(const RegionParams& copied)
        : lambdaIn(copied.lambdaIn)
        , lambdaOut(copied.lambdaOut)
    {
        this->normalize();
    }

    //! Copy assignement operator.
    RegionParams& operator=(const RegionParams& rhs)
    {
        this->lambdaIn = rhs.lambdaIn;
        this->lambdaOut = rhs.lambdaOut;

        this->normalize();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const RegionParams& lhs, const RegionParams& rhs)
    {
        return (lhs.lambdaIn == rhs.lambdaIn && lhs.lambdaOut == rhs.lambdaOut);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const RegionParams& lhs, const RegionParams& rhs)
    {
        return !(lhs == rhs);
    }

protected:
    //! Normalize value weight and returns the same value or a default value.
    static int normalize(int weight)
    {
        if (weight < 1)
            weight = 1;

        return weight;
    }

private:
    //! Check values of a configuration.
    void normalize()
    {
        lambdaIn = normalize(lambdaIn);
        lambdaOut = normalize(lambdaOut);
    }
};

enum class ColorSpaceOption
{
    RGB,
    YUV,
    Lab,
    Luv
};

inline constexpr const char* to_string(ColorSpaceOption clrOpt)
{
    switch (clrOpt)
    {
        case ColorSpaceOption::RGB:
            return "RGB";
        case ColorSpaceOption::YUV:
            return "YUV";
        case ColorSpaceOption::Lab:
            return "Lab";
        case ColorSpaceOption::Luv:
            return "Luv";
    }

    return "RGB";
}

inline ColorSpaceOption color_space_from_string(std::string_view s)
{
    if (s == "RGB")
        return ColorSpaceOption::RGB;
    if (s == "YUV")
        return ColorSpaceOption::YUV;
    if (s == "Lab")
        return ColorSpaceOption::Lab;
    if (s == "Luv")
        return ColorSpaceOption::Luv;

    return ColorSpaceOption::RGB;
}

//! \class RegionColorParams
//! Specific configuration for color region based active contour.
struct RegionColorParams : public RegionParams
{
    static constexpr ColorSpaceOption kDefaultColorSpace = ColorSpaceOption::RGB;

    static constexpr Components_3i kDefaultWeights{1, 1, 1};

    //! Color space option
    ColorSpaceOption color_space;

    //! Weights \a to calculate external speed \a Fd.
    Components_3i weights;

    //! Normalize values of a configuration.
    void normalize_region_color()
    {
        weights.c1 = normalize(weights.c1);
        weights.c2 = normalize(weights.c2);
        weights.c3 = normalize(weights.c3);
    }

    //! Default constructor.
    RegionColorParams()
        : RegionParams()
        , color_space(kDefaultColorSpace)
        , weights{kDefaultWeights}
    {
    }

    //! Copy constructor.
    RegionColorParams(const RegionColorParams& copied)
        : RegionParams(copied)
        , color_space(copied.color_space)
        , weights(copied.weights)
    {
        this->normalize_region_color();
    }

    //! Copy assignement operator.
    RegionColorParams& operator=(const RegionColorParams& rhs)
    {
        RegionParams::operator=(rhs);

        this->color_space = rhs.color_space;
        this->weights = rhs.weights;

        this->normalize_region_color();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const RegionColorParams& lhs, const RegionColorParams& rhs)
    {
        return (lhs.color_space == rhs.color_space && lhs.lambdaIn == rhs.lambdaIn &&
                lhs.lambdaOut == rhs.lambdaOut && lhs.weights == rhs.weights);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const RegionColorParams& lhs, const RegionColorParams& rhs)
    {
        return !(lhs == rhs);
    }
};

} // namespace fluvel_ip
