// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_data.hpp"
#include "point_containers.hpp"
#include "shape.hpp"

#include <limits>
#include <vector>

namespace ofeli_ip
{

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

//! \struct BoundarySwitchContext to perform generically a switch in or a switch out.
struct BoundarySwitchContext
{
    Contour& activeBoundary;
    Contour& adjacentBoundary;
    SpeedValue requiredSpeedSign;
    PhiValue currentToAdjacentVal;
    PhiValue neighborFromRegionVal;
    PhiValue neighbor_to_boundary_val;
    PhiValue redundantToRegionVal;

    static BoundarySwitchContext makeSwitchIn(ContourData& cd)
    {
        return {cd.l_out(),
                cd.l_in(),
                SpeedValue::GoOutward,
                PhiValue::InteriorBoundary,
                PhiValue::OutsideRegion,
                PhiValue::ExteriorBoundary,
                PhiValue::InsideRegion};
    }

    static BoundarySwitchContext makeSwitchOut(ContourData& cd)
    {
        return {cd.l_in(),
                cd.l_out(),
                SpeedValue::GoInward,
                PhiValue::ExteriorBoundary,
                PhiValue::InsideRegion,
                PhiValue::InteriorBoundary,
                PhiValue::OutsideRegion};
    }
};

//! \class EvolutionData
//! Holds the evolution data of the active contour.
struct EvolutionData
{
    //! Iterations number in a cycle (cycle 1 or cycle 2). It is set to 0 at the end of one
    //! cycle.
    int phaseStepCount{0};

    //! Total number of iterations the active contour has evolved from the initial contour.
    int stepCount{0};

    //! Maximum number of times the active contour can evolve.
    const int maxStepCount;

    //! Boolean egals to true if the active contour evolves in one way (at least) in cycle 1.
    bool isMoving{true};

    //! l_out shape at the end of the cycle 2.
    Shape l_out_shape;

    //! l_out shape at the end of the previous cycle 2.
    Shape previousShape;

    //! Total number of iterations the active contour has evolved from the initial contour
    //! at the end of the previous cycle 2.
    int previous_step_count{0};

    //! Hausdorff quantile
    //! at the end of the previous cycle 2.
    float previousQuantile{std::numeric_limits<float>::quiet_NaN()};

    //! Hausdorff quantile
    //! at the end of the cycle 2. It is a normalized value, divided by the diagonal size of
    //! #phi, in percent.
    float hausdorffQuantile{std::numeric_limits<float>::quiet_NaN()};

    //! Centroids gap between #l_out_shape and previousShape#. It is a normalized value,
    //! divided by the diagonal size of #phi, in percent.
    float centroidsDistance{std::numeric_limits<float>::quiet_NaN()};

    //! Intersection, i.e common points between #l_out_shape and #previousShape.
    PointSet intersection;

    //! Stopping condition status.
    StoppingStatus stoppingStatus{StoppingStatus::None};

    //! Constructor.
    EvolutionData(const ContourData& cd)
        : maxStepCount(5 * std::max(cd.phi().width(), cd.phi().height()))
    {
        l_out_shape.reserve(cd.l_out().capacity());
        previousShape.reserve(cd.l_out().capacity());
        intersection.reserve(cd.l_out().capacity());
    }

    //! Reset execution state of evolution data. Used for video tracking.
    void resetExecutionState()
    {
        phaseStepCount = 0;
        stepCount = 0;
        stoppingStatus = StoppingStatus::None;
    }
};

} // namespace ofeli_ip
