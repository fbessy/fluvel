// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter
{

/**
 * @brief Apply a 3x3 mean (box) filter using a preallocated output buffer.
 *
 * This function computes a separable 3x3 average filter on the input image
 * and writes the result into @p output.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 */
void mean3x3(const ImageView& input, ImageOwner& output);

/**
 * @brief Apply a 3x3 mean (box) filter and return a new image.
 *
 * Convenience wrapper allocating a new output image.
 *
 * @param input Input image view.
 * @return Filtered image.
 */
ImageOwner mean3x3(const ImageView& input);

/**
 * @brief Stateful 3x3 mean filter implementation.
 *
 * This class provides an efficient separable implementation of a 3x3
 * averaging filter using two passes (horizontal + vertical).
 *
 * Internal buffers are reused across calls, making it suitable for
 * repeated processing (e.g. video or pipelines).
 */
class Mean3x3
{
public:
    /**
     * @brief Initialize internal buffers from input image.
     *
     * Allocates or resizes intermediate buffers if needed.
     *
     * @param input Input image view.
     */
    void reset(const ImageView& input);

    /**
     * @brief Apply the 3x3 mean filter.
     *
     * Performs a horizontal pass followed by a vertical pass.
     *
     * @param input Input image view.
     */
    void apply(const ImageView& input);

    /**
     * @brief Get a non-owning view of the filtered result.
     *
     * @return ImageView referencing the internal buffer.
     */
    ImageView outputView() const;

    /**
     * @brief Get the filtered image with ownership.
     *
     * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output() const;

private:
    /**
     * @brief Horizontal 1D pass of the separable filter.
     *
     * Applies a 3-tap mean filter along rows.
     *
     * @param src Source row pointer.
     * @param dst Destination row pointer.
     * @param width Image width in pixels.
     * @param channels Number of channels per pixel.
     */
    void horizontalPass(const uint8_t* src, uint8_t* dst, int width, int channels);

    /**
     * @brief Vertical 1D pass of the separable filter.
     *
     * Combines three consecutive rows to produce the final result.
     *
     * @param row_m1 Row at y-1 (clamped if needed).
     * @param row_0 Row at y.
     * @param row_p1 Row at y+1 (clamped if needed).
     * @param dst Destination row pointer.
     * @param width Image width in pixels.
     * @param channels Number of channels per pixel.
     */
    void verticalPass(const uint8_t* row_m1, const uint8_t* row_0, const uint8_t* row_p1,
                      uint8_t* dst, int width, int channels);

    ImageOwner buffer1_; ///< Intermediate buffer (horizontal pass).
    ImageOwner buffer2_; ///< Output buffer (vertical pass).

    int width_{0};  ///< Cached image width.
    int height_{0}; ///< Cached image height.
};

} // namespace fluvel_ip::filter
