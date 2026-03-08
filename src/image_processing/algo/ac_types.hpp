// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "grid2d.hpp"

#include <string_view>
#include <vector>

namespace fluvel_ip
{

enum class PhiValue : int8_t
{
    InsideRegion = -3,
    InteriorBoundary = -1,
    ExteriorBoundary = 1,
    OutsideRegion = 3
};

using DiscreteLevelSet = Grid2D<PhiValue>;

enum class SpeedValue : int8_t
{
    GoInward = -1,
    NoMove = 0,
    GoOutward = 1
};

class ContourPoint
{
public:
    ContourPoint(int x, int y)
        : x_(x)
        , y_(y)
        , speed_(SpeedValue::NoMove)
    {
    }
    ContourPoint(const Point2D_i& p)
        : x_(p.x)
        , y_(p.y)
        , speed_(SpeedValue::NoMove)
    {
    }

    Point2D_i pos() const noexcept
    {
        return {x_, y_};
    }
    int x() const noexcept
    {
        return x_;
    }
    int y() const noexcept
    {
        return y_;
    }

    bool operator==(const ContourPoint& other) const noexcept
    {
        return x_ == other.x_ && y_ == other.y_;
    }

    bool operator!=(const ContourPoint& other) const noexcept
    {
        return !(*this == other);
    }

private:
    int x_;
    int y_;

    //! Pending sign speed of the algorithm to drive the contour
    SpeedValue speed_;

    friend class ActiveContour;
    friend class RegionAc;
    friend class RegionColorAc;
    friend class EdgeAc;
};

using Contour = std::vector<ContourPoint>;
using ExportedContour = std::vector<Point2D_i>;

//! Pixel connectivity of the neighborhood used by the algorithm (4- or 8-connected).
enum class Connectivity
{
    Four,
    Eight
};

inline constexpr const char* to_string(Connectivity connectivity)
{
    switch (connectivity)
    {
        case Connectivity::Four:
            return "4";
        case Connectivity::Eight:
            return "8";
    }

    return "4";
}

inline Connectivity connectivity_from_string(std::string_view c)
{
    if (c == "4")
        return Connectivity::Four;
    if (c == "8")
        return Connectivity::Eight;

    return Connectivity::Four;
}

//! Defines how the algorithm handles degraded or failure cases.
//! Typically used for image segmentation (StopOnFailure)
//! and video tracking (RecoverOnFailure).
enum class FailureHandlingMode
{
    StopOnFailure,   //!< Intended for image segmentation.
    RecoverOnFailure //!< Intended for video tracking.
};

//! \enum Stopping condition status.
enum class StoppingStatus
{
    None,           //!< the active contour is not stopped.
    ListsConverged, //!< speed <= 0 for all points of Lout and speed >= 0 for all points of Lin
    Hausdorff,      //!< Hausorff distance fallback, available only in StopOnFailure mode.
    MaxIteration,   //!< Maximum number of elementary steps reached, last fallback to avoid
    //!  infinite loop of the method converge().
    EmptyListFailure //!< One or the both lists is/are empty. The definition of both contiguous
                     //!< lists
    //!  as a boundary is not respected.
};

//! Internal phase state of the active contour.
//! A logical cycle consists of Phase Cycle1 followed by Phase Cycle2,
//! as described in the reference paper.
enum class PhaseState
{
    Cycle1,      //!< Phase 1 (called "Cycle 1" in the reference paper)
    Cycle2,      //!< Phase 2 (called "Cycle 2" in the reference paper)
    FinalCycle2, //!< Final Phase 2 before termination
    Stopped
};

//! BoundarySwitch to perform switch_in or switch_out procedures generically.
enum class BoundarySwitch
{
    In,
    Out
};

//! \class AcConfig
//! Active contour configuration
struct AcConfig
{
    static constexpr bool kDefaultIsCycle2 = true;
    static constexpr int kDefaultDiskRadius = 2;
    static constexpr int kDefaultNa = 30;
    static constexpr int kDefaultNs = 3;
    static constexpr FailureHandlingMode kDefaultFailureMode = FailureHandlingMode::StopOnFailure;

    //! Boolean egals to \c true to have the curve smoothing, evolutions in the cycle 2 with the
    //! internal speed Fint.
    bool hasCycle2;

    //!  Disk radius for the curve smoothing.
    int diskRadius;

    //! Maximum number of times the active contour can evolve in a cycle 1 with \a Fd speed.
    int Na;

    //! Maximum number of times the active contour can evolve in a cycle 2 with \a Fint speed.
    int Ns;

    ///! Defines how the algorithm handles degraded or failure cases.
    FailureHandlingMode failureMode;

    //! Normalize values of a configuration.
    void normalize()
    {
        if (diskRadius < 1)
            diskRadius = 1;

        if (Na < 1)
            Na = 1;

        if (Ns < 1)
            Ns = 1;
    }

    //! Default constructor.
    AcConfig()
        : hasCycle2(kDefaultIsCycle2)
        , diskRadius(kDefaultDiskRadius)
        , Na(kDefaultNa)
        , Ns(kDefaultNs)
        , failureMode(kDefaultFailureMode)
    {
    }

    //! Copy constructor.
    AcConfig(const AcConfig& copied)
        : hasCycle2(copied.hasCycle2)
        , diskRadius(copied.diskRadius)
        , Na(copied.Na)
        , Ns(copied.Ns)
        , failureMode(copied.failureMode)
    {
        this->normalize();
    }

    //! Copy assignement operator.
    AcConfig& operator=(const AcConfig& rhs)
    {
        this->hasCycle2 = rhs.hasCycle2;
        this->diskRadius = rhs.diskRadius;
        this->Na = rhs.Na;
        this->Ns = rhs.Ns;
        this->failureMode = rhs.failureMode;

        this->normalize();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const AcConfig& lhs, const AcConfig& rhs)
    {
        return (lhs.hasCycle2 == rhs.hasCycle2 && lhs.diskRadius == rhs.diskRadius &&
                lhs.Na == rhs.Na && lhs.Ns == rhs.Ns && lhs.failureMode == rhs.failureMode);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const AcConfig& lhs, const AcConfig& rhs)
    {
        return !(lhs == rhs);
    }
};

//! \class RegionConfig
//! Specific configuration for region based active contour
struct RegionConfig
{
    static constexpr int kDefaultLambdaIn = 1;
    static constexpr int kDefaultLambdaOut = 1;

    //! Weight of the inside homogeneity criterion in the Chan-Vese model
    //! (called lambda 1 in the article "Active contour without edges.").
    int lambdaIn;

    //! Weight of the outside homogeneity criterion in the Chan-Vese model
    //! (called lambda 2 in the article "Active contour without edges.").
    int lambdaOut;

    //! Check values of a configuration.
    void normalize()
    {
        lambdaIn = normalize(lambdaIn);
        lambdaOut = normalize(lambdaOut);
    }

    //! Default constructor.
    RegionConfig()
        : lambdaIn(kDefaultLambdaIn)
        , lambdaOut(kDefaultLambdaOut)
    {
    }

    //! Destructor.
    virtual ~RegionConfig() = default;

    //! Copy constructor.
    RegionConfig(const RegionConfig& copied)
        : lambdaIn(copied.lambdaIn)
        , lambdaOut(copied.lambdaOut)
    {
        this->normalize();
    }

    //! Copy assignement operator.
    RegionConfig& operator=(const RegionConfig& rhs)
    {
        this->lambdaIn = rhs.lambdaIn;
        this->lambdaOut = rhs.lambdaOut;

        this->normalize();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const RegionConfig& lhs, const RegionConfig& rhs)
    {
        return (lhs.lambdaIn == rhs.lambdaIn && lhs.lambdaOut == rhs.lambdaOut);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const RegionConfig& lhs, const RegionConfig& rhs)
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

//! \class RegionColorConfig
//! Specific configuration for color region based active contour.
struct RegionColorConfig : public RegionConfig
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
    RegionColorConfig()
        : RegionConfig()
        , color_space(kDefaultColorSpace)
        , weights{kDefaultWeights}
    {
    }

    ~RegionColorConfig() override = default;

    //! Copy constructor.
    RegionColorConfig(const RegionColorConfig& copied)
        : RegionConfig(copied)
        , color_space(copied.color_space)
        , weights(copied.weights)
    {
        this->normalize_region_color();
    }

    //! Copy assignement operator.
    RegionColorConfig& operator=(const RegionColorConfig& rhs)
    {
        RegionConfig::operator=(rhs);

        this->color_space = rhs.color_space;
        this->weights = rhs.weights;

        this->normalize_region_color();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const RegionColorConfig& lhs, const RegionColorConfig& rhs)
    {
        return (lhs.color_space == rhs.color_space && lhs.lambdaIn == rhs.lambdaIn &&
                lhs.lambdaOut == rhs.lambdaOut && lhs.weights == rhs.weights);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const RegionColorConfig& lhs, const RegionColorConfig& rhs)
    {
        return !(lhs == rhs);
    }
};

//! Kernel support to know the geometry limit of the internal kernel.
struct KernelSupport
{
    int min_dx = 0;
    int max_dx = 0;
    int min_dy = 0;
    int max_dy = 0;
};

//! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
struct InternalKernel
{
    std::vector<int> offsets;
    KernelSupport support;

    bool fully_inside(int x, int y, int width, int height) const
    {
        return x + support.min_dx >= 0 && x + support.max_dx < width && y + support.min_dy >= 0 &&
               y + support.max_dy < height;
    }
};

} // namespace fluvel_ip
