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
     * @param firstFrame First input frame.
     */
    void reset(const ImageView& firstFrame);

    /**
     * @brief Update the filter with a new frame.
     *
     * Updates the temporal accumulation using the new input frame.
     *
     * @param frame Current frame.
     */
    void update(const ImageView& frame);

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
     * @brief Estimate scene motion from the current frame.
     *
     * Computes the root mean square (RMS) color difference between the
     * current frame and the temporal accumulation buffer.
     *
     * A spatial subsampling strategy is used to reduce computation cost.
     *
     * @param frame Current input frame.
     * @return Estimated motion magnitude.
     */
    [[nodiscard]]
    float computeMotion(const ImageView& frame) const;

    /**
     * @brief Update internal noise estimation.
     *
     * Adjusts filter parameters based on motion and elapsed time.
     *
     * @param motion Estimated scene motion.
     * @param deltaSeconds Time delta since last update.
     */
    void updateNoiseEstimate(float motion, float deltaSeconds);

    /**
     * @brief Compute the adaptive temporal smoothing coefficient.
     *
     * Converts the estimated scene motion into an EWMA coefficient
     * using the current noise estimate and a continuous-time model.
     *
     * @param motion Estimated scene motion.
     * @param deltaSeconds Time elapsed since the previous update.
     * @return Adaptive smoothing coefficient in the range [0, 1].
     */
    [[nodiscard]]
    float computeAdaptiveAlpha(float motion, float deltaSeconds);

    /**
     * @brief Apply exponential temporal smoothing to the accumulation buffer.
     *
     * Updates the internal accumulation image using the current input frame
     * and the specified smoothing coefficient.
     *
     * Each pixel is updated according to an exponentially weighted moving
     * average (EWMA):
     *
     * accum = accum + alpha * (frame - accum)
     *
     * @param frame Current input frame.
     * @param alpha Smoothing coefficient in the range [0, 1].
     */
    void applyTemporalSmoothing(const ImageView& frame, float alpha);

    /**
     * @brief Update the output image from the accumulation buffer.
     */
    void updateOutput();

    /// Accumulated floating-point image (temporal average).
    Grid2D<Rgb_f> accum_;

    /// Spatial sampling step (used for motion estimation).
    int samplingStep_{1};

    /// Estimated noise level.
    float noiseEstimate_{0.f};

    /// Indicates whether a valid temporal history is available.
    bool hasTemporalHistory_{false};

    /// Timer used to compute time differences between frames.
    ElapsedTimer deltaFrameTimer_;

    /// Timer used during the filter warm-up phase.
    ElapsedTimer warmupTimer_;

    /// Output image buffer.
    ImageOwner output_;

    /// Smoothed motion-to-noise ratio used to stabilize
    /// the adaptive smoothing coefficient.
    float motionRatioFiltered_{1.f};

    /// Indicates whether the cached output image must be regenerated.
    bool outputNeedsRefresh_{true};
};

} // namespace fluvel_ip::filter
