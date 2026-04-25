// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <compare>

namespace fluvel_ip
{

/**
 * @brief Defines how the algorithm handles degraded or failure cases.
 *
 * This mode controls the behavior of the active contour when encountering
 * unstable or invalid states. It is typically used to distinguish between:
 * - image segmentation (strict behavior)
 * - video tracking (more tolerant behavior)
 */
enum class FailureHandlingMode
{
    StopOnFailure, //!< Stop the algorithm immediately when a failure occurs (recommended for static
                   //!< image segmentation).
    RecoverOnFailure //!< Attempt to recover from failure and continue processing (recommended for
                     //!< video tracking).
};

/**
 * @brief Describes the stopping condition of the active contour algorithm.
 *
 * This status indicates why the evolution of the contour has stopped.
 */
enum class StoppingStatus
{
    None,           //!< The active contour is still evolving.
    ListsConverged, //!< Convergence reached: all Lout points have speed <= 0 and all Lin points
                    //!< have speed >= 0.
    Hausdorff,      //!< Hausdorff distance fallback (available only in StopOnFailure mode).
    MaxIteration, //!< Maximum number of iterations reached (safety stop to prevent infinite loops).
    EmptyListFailure, //!< One or both contour lists are empty, violating the boundary definition.
    SpeedModelFailure //!< Failure in the speed model (initialization or runtime error).
};

/**
 * @brief Internal phase state of the active contour algorithm.
 *
 * A complete iteration typically consists of:
 * - Cycle1 (data-driven evolution)
 * - Cycle2 (regularization/smoothing)
 */
enum class PhaseState
{
    Cycle1,      //!< First phase ("Cycle 1" in the reference paper).
    Cycle2,      //!< Second phase ("Cycle 2" in the reference paper).
    FinalCycle2, //!< Final smoothing phase before termination.
    Stopped      //!< Algorithm has stopped.
};

/**
 * @brief Direction used for switching operations in the contour.
 *
 * This is used to generalize inward/outward movements of the contour.
 */
enum class SwitchDirection
{
    In, //!< Outside → inside transition (outward expansion of the contour).
    Out //!< Inside → outside transition (inward contraction of the contour).
};

/**
 * @brief Configuration parameters for the active contour algorithm.
 *
 * This structure defines all tunable parameters controlling the contour evolution,
 * including iteration limits, smoothing behavior, and failure handling.
 *
 * All values are normalized to valid ranges after construction or assignment.
 */
struct ActiveContourParams
{
    static constexpr bool kDefaultIsCycle2 = true; //!< Default: enable smoothing cycle.
    static constexpr int kDefaultDiskRadius = 2;   //!< Default smoothing radius.
    static constexpr int kDefaultNa = 30;          //!< Default max iterations for Cycle1.
    static constexpr int kDefaultNs = 3;           //!< Default max iterations for Cycle2.
    static constexpr FailureHandlingMode kDefaultFailureMode =
        FailureHandlingMode::StopOnFailure; //!< Default failure handling mode.

    /**
     * @brief Enables smoothing evolution (Cycle 2).
     *
     * When true, the contour performs an additional smoothing phase using
     * the internal speed (Fint).
     */
    bool cycle2Enabled;

    /**
     * @brief Disk radius used for contour smoothing.
     *
     * Must be >= 1. Larger values produce stronger smoothing.
     */
    int diskRadius;

    /**
     * @brief Maximum number of iterations for Cycle1 (data-driven evolution).
     *
     * Must be >= 1.
     */
    int Na;

    /**
     * @brief Maximum number of iterations for Cycle2 (smoothing phase).
     *
     * Must be >= 1.
     */
    int Ns;

    /**
     * @brief Defines how failures are handled during execution.
     */
    FailureHandlingMode failureMode;

    /**
     * @brief Normalizes parameter values to valid ranges.
     *
     * Ensures that all parameters satisfy minimal constraints:
     * - diskRadius >= 1
     * - Na >= 1
     * - Ns >= 1
     */
    void normalize()
    {
        if (diskRadius < 1)
            diskRadius = 1;

        if (Na < 1)
            Na = 1;

        if (Ns < 1)
            Ns = 1;
    }

    /**
     * @brief Default constructor.
     *
     * Initializes parameters with default values and normalizes them.
     */
    ActiveContourParams()
        : cycle2Enabled(kDefaultIsCycle2)
        , diskRadius(kDefaultDiskRadius)
        , Na(kDefaultNa)
        , Ns(kDefaultNs)
        , failureMode(kDefaultFailureMode)
    {
        this->normalize();
    }

    /**
     * @brief Copy constructor.
     *
     * Copies all parameters and normalizes them.
     *
     * @param copied Source parameters.
     */
    ActiveContourParams(const ActiveContourParams& copied)
        : cycle2Enabled(copied.cycle2Enabled)
        , diskRadius(copied.diskRadius)
        , Na(copied.Na)
        , Ns(copied.Ns)
        , failureMode(copied.failureMode)
    {
        this->normalize();
    }

    /**
     * @brief Copy assignment operator.
     *
     * Assigns all parameters and normalizes them.
     *
     * @param rhs Source parameters.
     * @return Reference to this instance.
     */
    ActiveContourParams& operator=(const ActiveContourParams& rhs)
    {
        this->cycle2Enabled = rhs.cycle2Enabled;
        this->diskRadius = rhs.diskRadius;
        this->Na = rhs.Na;
        this->Ns = rhs.Ns;
        this->failureMode = rhs.failureMode;

        this->normalize();

        return *this;
    }

    /**
     * @brief Default comparison operator (C++20).
     *      * Generates all comparison operators (==, !=, <, <=, >, >=)
     * by comparing members in declaration order.
     *      * @note Requires <compare>.
     */
    auto operator<=>(const ActiveContourParams&) const = default;
};

} // namespace fluvel_ip
