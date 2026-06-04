// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "grid2d.hpp"
#include "image_owner.hpp"
#include "image_view.hpp"

#include "elapsed_timer.hpp"

namespace fluvel_ip::filter
{

/**
 * @brief Adaptive temporal smoothing filter.
 *
 * This filter performs temporal denoising by maintaining an
 * exponentially weighted moving average (EWMA) of successive frames.
 *
 * The smoothing strength is automatically adjusted according to the
 * estimated scene motion and noise level:
 *
 * - static scenes are strongly averaged to reduce sensor noise,
 * - moving scenes remain reactive to avoid motion blur and ghosting.
 *
 * The implementation uses a continuous-time formulation, making the
 * filter largely independent of the input frame rate.
 *
 * Typical use cases:
 * - video denoising,
 * - low-light image stabilization,
 * - temporal noise reduction in live acquisition pipelines.
 */
class TemporalMean
{
public:
    /**
     * @brief Initialize the filter with the first frame.
     *
     * Allocates internal buffers and initializes accumulation state.
     *
     * @param first_src First input frame.
     */
    void reset(const ImageView& first_src);

    /**
     * @brief Update the filter with a new frame.
     *
     * Updates the temporal accumulation using the new input frame.
     *
     * @param src Current frame.
     */
    void update(const ImageView& src);

    /**
     * @brief Get the accumulated floating-point buffer.
     *
     * @return Internal accumulation buffer (RGB float).
     */
    [[nodiscard]]
    const Grid2D<Rgb_f>& accum() const
    {
        return accum_;
    }

    /**
     * @brief Get a non-owning view of the filtered output.
     *
     * @return ImageView referencing the internal output buffer.
     * @warning The returned view is valid as long as the object is alive.
     */
    [[nodiscard]]
    ImageView outputView();

    /**
     * @brief Get the filtered output image.
     *
     * @return Reference to the internal ImageOwner.
     */
    [[nodiscard]]
    const ImageOwner& output();

private:
    /**
     * @brief Update internal noise estimation.
     *
     * Adjusts filter parameters based on motion and elapsed time.
     *
     * @param motion Estimated motion ratio.
     * @param deltaSeconds Time delta since last update.
     */
    void updateNoiseEstimate(float motion, float deltaSeconds);

    /**
     * @brief Update the output image from the accumulation buffer.
     */
    void updateOutput();

    /// Accumulated floating-point image (temporal average).
    Grid2D<Rgb_f> accum_;

    /// Indicates whether the filter has been initialized.
    bool accumInit_{false};

    /// Spatial sampling step (used for motion estimation).
    int samplingStep_{1};

    /// Estimated noise level.
    float noiseEstimate_{0.f};

    /// Indicates whether noise estimation has been initialized.
    bool noiseEstimInit_{false};

    /// Indicates if at least one frame was processed after the reset.
    bool hasPreviousFrame_{false};

    /// Timer used to compute time differences between frames.
    ElapsedTimer deltaFrameTimer_;

    /// Timer used to compute elapsed time after a reset.
    ElapsedTimer afterResetTimer_;

    /// Output image buffer.
    ImageOwner output_;

    /// Smoothed motion ratio.
    float motionRatioFiltered_{1.f};

    /// Indicates whether the cached output image must be regenerated.
    bool outputNeedsRefresh_{true};
};

} // namespace fluvel_ip::filter
