// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

class ContourPoint;
class ContourData;
struct ContourDiagnostics;
enum class SwitchDirection;

/**
 * @brief Status of a speed model.
 *
 * Used to report initialization or runtime failures during contour evolution.
 */
enum class SpeedModelStatus
{
    Ok,            ///< Model is valid and operational.
    InitFailed,    ///< Initialization failed (e.g. invalid data).
    RuntimeFailure ///< Runtime error occurred during evolution.
};

/**
 * @brief Interface for speed models used in active contour evolution.
 *
 * A speed model defines how the contour evolves by computing a speed value
 * for each contour point. It can optionally react to lifecycle events such as:
 * - image change
 * - iteration steps
 * - topology changes (switch in/out)
 *
 * This interface enables interchangeable models (e.g. grayscale, color, texture).
 */
class ISpeedModel
{
public:
    /// Virtual destructor.
    virtual ~ISpeedModel() = default;

    /**
     * @brief Called when a new input image is provided.
     *      * This method can be used to initialize or update internal state
     * (e.g. region statistics, histograms, precomputed terms).
     *      * ⚠️ Lifetime requirement:
     * The provided ImageView may be stored and reused by the implementation.
     * The underlying image data MUST remain valid for the entire duration
     * of its usage (typically until the next call to onImageChanged()).
     *      * Passing a temporary or short-lived ImageView may lead to undefined behavior.
     * Use an ImageOwner to guarantee safe lifetime when needed.
     *      * @param image Input image.
     * @param contour Current contour data.
     */
    virtual void onImageChanged(const ImageView&, const ContourData&)
    {
    }

    /**
     * @brief Called at the beginning of each Cycle 1 iteration.
     *
     * Can be used to update internal statistics incrementally.
     */
    virtual void onStepCycle1()
    {
    }

    /**
     * @brief Compute speed for a contour point.
     *
     * This is the core method of the speed model.
     *
     * @param point Contour point to update.
     * @param phi Discrete level-set function.
     */
    virtual void computeSpeed(ContourPoint&, const DiscreteLevelSet& phi) = 0;

    /**
     * @brief Called when a point switches region (in/out).
     *
     * Useful to update incremental statistics.
     *
     * @param point Contour point involved in the switch.
     * @param direction Direction of the switch.
     */
    virtual void onSwitch(const ContourPoint&, SwitchDirection)
    {
    }

    /**
     * @brief Fill diagnostic information.
     *
     * @param diagnostics Output diagnostics structure.
     */
    virtual void fillDiagnostics(ContourDiagnostics&) const
    {
    }

    /**
     * @brief Get current model status.
     *
     * @return Status of the model.
     */
    virtual SpeedModelStatus status() const
    {
        return status_;
    }

protected:
    /// Internal status of the model.
    SpeedModelStatus status_{SpeedModelStatus::Ok};
};

} // namespace fluvel_ip
