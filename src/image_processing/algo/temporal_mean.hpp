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
 * @brief Temporal mean filter with adaptive behavior.
 *
 * This filter accumulates successive frames to reduce noise over time.
 * It uses an exponential moving average and adapts its behavior based
 * on motion estimation and temporal dynamics.
 *
 * Typical use case:
 * - video denoising
 * - stabilization of noisy signals
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
    ImageView outputView();

    /**
     * @brief Get the filtered output image.
     *
     * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output();

private:
    /**
     * @brief Update internal noise estimation.
     *
     * Adjusts filter parameters based on motion and elapsed time.
     *
     * @param motion Estimated motion ratio.
     * @param dt_seconds Time delta since last update.
     */
    void updateNoiseEstimate(float motion, float dt_seconds);

    /**
     * @brief Update the output image from the accumulation buffer.
     */
    void updateOutput();

    /// Accumulated floating-point image (temporal average).
    Grid2D<Rgb_f> accum_;

    /// Indicates whether the filter has been initialized.
    bool initialized_ = false;

    /// Spatial sampling step (used for motion estimation).
    int sampling_step_ = 1;

    /// Exponential smoothing factor.
    float alpha_ = 0.1f;

    /// Estimated noise level.
    float noise_estimate_;

    /// Indicates whether noise estimation has been initialized.
    bool noise_initialized_ = false;

    /// Indicates whether timing has been initialized.
    bool time_initialized_ = false;

    /// Timer used to compute time differences between frames.
    ElapsedTimer timer_;

    /// Time elapsed since initialization (seconds).
    float time_since_init_;

    /// Indicates whether high motion is detected.
    bool high_motion_ = false;

    /// Output image buffer.
    ImageOwner output_;

    /// Smoothed motion ratio.
    float motion_ratio_filtered_ = 1.f;
};

} // namespace fluvel_ip::filter
