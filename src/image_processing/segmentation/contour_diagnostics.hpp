// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_types.hpp"

#include <cstddef>
#include <vector>

namespace fluvel_ip
{

/**
 * @brief Small utility container representing a multi-channel value.
 *
 * This class is used to represent scalar or vector quantities such as
 * grayscale intensity (1 channel) or RGB color (3 channels).
 */
class ChannelVector
{
public:
    /**
     * @brief Construct a single-channel vector.
     *
     * @param v Scalar value.
     */
    ChannelVector(int v)
    {
        values_ = {v};
    }

    /**
     * @brief Construct a 3-channel vector (e.g. RGB).
     *
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     */
    ChannelVector(int r, int g, int b)
    {
        values_ = {r, g, b};
    }

    /**
     * @brief Get the number of channels.
     *
     * @return Dimension of the vector.
     */
    size_t dimension() const
    {
        return values_.size();
    }

    /**
     * @brief Access the underlying values.
     *
     * @return Const reference to channel values.
     */
    const std::vector<int>& values() const
    {
        return values_;
    }

private:
    std::vector<int> values_; ///< Channel values (1 or 3 typically).
};

/**
 * @brief Diagnostic information for an active contour execution.
 *
 * This structure gathers runtime metrics useful for:
 * - debugging,
 * - monitoring convergence,
 * - displaying UI feedback.
 */
struct ContourDiagnostics
{
    int stepCount = 0; ///< Total number of evolution steps performed.

    fluvel_ip::PhaseState state{fluvel_ip::PhaseState::Cycle1}; ///< Current phase of the algorithm.

    ChannelVector meanInside{0, 0, 0}; ///< Mean value inside the contour (per channel).

    ChannelVector meanOutside{0, 0, 0}; ///< Mean value outside the contour (per channel).

    fluvel_ip::StoppingStatus stoppingStatus{
        fluvel_ip::StoppingStatus::None}; ///< Reason for stopping.

    float hausdorffQuantile = 0.f; ///< Normalized Hausdorff distance (quantile).

    float relativeCentroidDistance = 0.f; ///< Normalized centroid displacement.

    double elapsedSec = 0.0; ///< Execution time in seconds.

    std::size_t contourSize = 0; ///< Number of points in the contour.
};

/**
 * @brief Convert PhaseState to a human-readable string.
 *
 * @param state Phase state.
 * @return String representation.
 */
inline const char* toString(PhaseState state)
{
    switch (state)
    {
        case PhaseState::Cycle1:
            return "Cycle 1";
        case PhaseState::Cycle2:
            return "Cycle 2";
        case PhaseState::FinalCycle2:
            return "Final Cycle 2";
        case PhaseState::Stopped:
            return "Stopped";
    }

    return "Unknown";
}

/**
 * @brief Convert StoppingStatus to a human-readable string.
 *
 * @param status Stopping condition.
 * @return String representation.
 */
inline const char* toString(StoppingStatus status)
{
    switch (status)
    {
        case StoppingStatus::None:
            return "None";
        case StoppingStatus::ListsConverged:
            return "ListsConverged";
        case StoppingStatus::Hausdorff:
            return "Hausdorff";
        case StoppingStatus::MaxIteration:
            return "MaxIteration";
        case StoppingStatus::EmptyListFailure:
            return "EmptyListFailure";
        case StoppingStatus::SpeedModelFailure:
            return "SpeedModelFailure";
    }

    return "Unknown";
}

} // namespace fluvel_ip
