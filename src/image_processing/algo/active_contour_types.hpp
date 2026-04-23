// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_ip
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
    None,             //!< the active contour is not stopped.
    ListsConverged,   //!< speed <= 0 for all points of Lout and speed >= 0 for all points of Lin
    Hausdorff,        //!< Hausorff distance fallback, available only in StopOnFailure mode.
    MaxIteration,     //!< Maximum number of elementary steps reached, last fallback to avoid
                      //!  infinite loop of the method converge().
    EmptyListFailure, //!< One or the both lists is/are empty. The definition of both contiguous
                      //!< lists as a boundary is not respected.
    SpeedModelFailure //!< Speed model failure (init or runtime speed error).
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

//! SwitchDirection to perform switch_in or switch_out procedures generically.
enum class SwitchDirection
{
    In, // outside -> inside (=> outward move)
    Out // inside -> outside (=> inward move)
};

//! \class ActiveContourParams
//! Active contour configuration
struct ActiveContourParams
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
    ActiveContourParams()
        : hasCycle2(kDefaultIsCycle2)
        , diskRadius(kDefaultDiskRadius)
        , Na(kDefaultNa)
        , Ns(kDefaultNs)
        , failureMode(kDefaultFailureMode)
    {
        this->normalize();
    }

    //! Copy constructor.
    ActiveContourParams(const ActiveContourParams& copied)
        : hasCycle2(copied.hasCycle2)
        , diskRadius(copied.diskRadius)
        , Na(copied.Na)
        , Ns(copied.Ns)
        , failureMode(copied.failureMode)
    {
        this->normalize();
    }

    //! Copy assignement operator.
    ActiveContourParams& operator=(const ActiveContourParams& rhs)
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
    friend bool operator==(const ActiveContourParams& lhs, const ActiveContourParams& rhs)
    {
        return (lhs.hasCycle2 == rhs.hasCycle2 && lhs.diskRadius == rhs.diskRadius &&
                lhs.Na == rhs.Na && lhs.Ns == rhs.Ns && lhs.failureMode == rhs.failureMode);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const ActiveContourParams& lhs, const ActiveContourParams& rhs)
    {
        return !(lhs == rhs);
    }
};

} // namespace fluvel_ip
