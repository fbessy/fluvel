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
 * @brief Parameters controlling the active contour evolution.
 *
 * This structure defines all parameters used during the evolution process,
 * including data-driven iterations (Cycle 1) and optional smoothing (Cycle 2).
 *
 * Default values are provided via in-class member initialization.
 * Instances can safely be value-initialized using {}.
 *
 * @note All parameters are expected to be valid when used.
 *       Validation should be handled by the caller (UI, config system, etc.).
 */
struct ActiveContourParams
{
    /// Default: enable smoothing phase (Cycle 2).
    static constexpr bool kDefaultCycle2Enabled{true};

    /// Default smoothing disk radius.
    static constexpr int kDefaultDiskRadius{2};

    /// Default maximum iterations for Cycle 1 (data-driven evolution).
    static constexpr int kDefaultNa{30};

    /// Default maximum iterations for Cycle 2 (smoothing phase).
    static constexpr int kDefaultNs{3};

    /// Default failure handling strategy.
    static constexpr FailureHandlingMode kDefaultFailureMode{FailureHandlingMode::StopOnFailure};

    /**
     * @brief Enables smoothing evolution (Cycle 2).
     *
     * When true, the contour performs an additional smoothing phase
     * using the internal speed (Fint).
     */
    bool cycle2Enabled{kDefaultCycle2Enabled};

    /**
     * @brief Disk radius used for contour smoothing.
     *
     * Must be >= 1. Larger values produce stronger smoothing.
     */
    int diskRadius{kDefaultDiskRadius};

    /**
     * @brief Maximum number of iterations for Cycle 1.
     *
     * Controls the data-driven evolution phase.
     * Must be >= 1.
     */
    int Na{kDefaultNa};

    /**
     * @brief Maximum number of iterations for Cycle 2.
     *
     * Controls the smoothing phase.
     * Must be >= 1.
     */
    int Ns{kDefaultNs};

    /**
     * @brief Defines how failures are handled during execution.
     */
    FailureHandlingMode failureMode{kDefaultFailureMode};

    /// Default comparison operator (C++20).
    auto operator<=>(const ActiveContourParams&) const = default;
};

} // namespace fluvel_ip
