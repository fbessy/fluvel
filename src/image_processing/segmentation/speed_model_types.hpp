// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"

#include <compare>
#include <string_view>

namespace fluvel_ip
{

/**
 * @brief Parameters for region-based active contour (Chan-Vese model).
 *
 * This structure defines the weights associated with the homogeneity terms
 * inside and outside the contour.
 */
struct RegionParams
{
    /// Default weight for inside region.
    static constexpr int kDefaultLambdaIn{1};

    /// Default weight for outside region.
    static constexpr int kDefaultLambdaOut{1};

    /**
     * @brief Weight of the inside homogeneity term (λ₁ in Chan-Vese).
     */
    int lambdaIn;

    /**
     * @brief Weight of the outside homogeneity term (λ₂ in Chan-Vese).
     */
    int lambdaOut;

    /**
     * @brief Default constructor.
     */
    RegionParams()
        : lambdaIn(kDefaultLambdaIn)
        , lambdaOut(kDefaultLambdaOut)
    {
    }

    /**
     * @brief Destructor.
     *
     * Non-virtual on purpose: this is a value-type configuration struct.
     */
    ~RegionParams() = default;

    /**
     * @brief Copy constructor.
     */
    RegionParams(const RegionParams& copied)
        : lambdaIn(copied.lambdaIn)
        , lambdaOut(copied.lambdaOut)
    {
        this->normalize();
    }

    /**
     * @brief Copy assignment operator.
     */
    RegionParams& operator=(const RegionParams& rhs)
    {
        this->lambdaIn = rhs.lambdaIn;
        this->lambdaOut = rhs.lambdaOut;

        this->normalize();

        return *this;
    }

    /**
     * @brief Default comparison operator (C++20).
     *      * Generates all comparison operators (==, !=, <, <=, >, >=)
     * by comparing members in declaration order.
     *      * @note Requires <compare>.
     */
    auto operator<=>(const RegionParams&) const = default;

protected:
    /**
     * @brief Normalize a weight value.
     *
     * Ensures that weights are strictly positive.
     *
     * @param weight Input weight.
     * @return Normalized weight.
     */
    static int normalize(int weight)
    {
        if (weight < 1)
            weight = 1;

        return weight;
    }

private:
    /**
     * @brief Normalize all parameters.
     */
    void normalize()
    {
        lambdaIn = normalize(lambdaIn);
        lambdaOut = normalize(lambdaOut);
    }
};

/**
 * @brief Supported color spaces for region-based models.
 */
enum class ColorSpaceOption
{
    RGB, ///< Red-Green-Blue
    YUV, ///< Luminance-Chrominance
    Lab, ///< CIE Lab color space
    Luv  ///< CIE Luv color space
};

/**
 * @brief Convert ColorSpaceOption to string.
 */
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

/**
 * @brief Convert string to ColorSpaceOption.
 *
 * Defaults to RGB if the string is not recognized.
 */
inline ColorSpaceOption colorSpaceFromString(std::string_view s)
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

/**
 * @brief Parameters for color-based region active contour.
 *
 * Extends RegionParams by adding:
 * - color space selection
 * - per-channel weighting
 *
 * Used in multi-channel Chan-Vese models.
 */
struct RegionColorParams : public RegionParams
{
    /// Default color space.
    static constexpr ColorSpaceOption kDefaultColorSpace{ColorSpaceOption::RGB};

    /// Default channel weights.
    static constexpr Components_3i kDefaultWeights{1, 1, 1};

    /**
     * @brief Color space used for computation.
     */
    ColorSpaceOption colorSpace;

    /**
     * @brief Per-channel weights for the energy computation.
     *
     * These weights control the influence of each channel (e.g. R/G/B or Y/U/V).
     */
    Components_3i weights;

    /**
     * @brief Normalize channel weights.
     */
    void normalizeColorParams()
    {
        weights.c1 = normalize(weights.c1);
        weights.c2 = normalize(weights.c2);
        weights.c3 = normalize(weights.c3);
    }

    /**
     * @brief Default constructor.
     */
    RegionColorParams()
        : RegionParams()
        , colorSpace(kDefaultColorSpace)
        , weights{kDefaultWeights}
    {
    }

    /**
     * @brief Copy constructor.
     */
    RegionColorParams(const RegionColorParams& copied)
        : RegionParams(copied)
        , colorSpace(copied.colorSpace)
        , weights(copied.weights)
    {
        this->normalizeColorParams();
    }

    /**
     * @brief Copy assignment operator.
     */
    RegionColorParams& operator=(const RegionColorParams& rhs)
    {
        RegionParams::operator=(rhs);

        this->colorSpace = rhs.colorSpace;
        this->weights = rhs.weights;

        this->normalizeColorParams();

        return *this;
    }

    /**
     * @brief Default comparison operator (C++20).
     *      * Generates all comparison operators (==, !=, <, <=, >, >=)
     * by comparing members in declaration order.
     *      * @note Requires <compare>.
     */
    auto operator<=>(const RegionColorParams&) const = default;
};

} // namespace fluvel_ip
