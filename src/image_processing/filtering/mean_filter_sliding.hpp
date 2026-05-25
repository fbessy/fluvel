// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter
{

/**
 * @brief Apply a mean (box) filter using a sliding window with a preallocated output buffer.
 *
 * This function computes a mean filter of radius @p radius using an efficient
 * sliding window approach (O(N)). The result is written into @p output.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param radius Radius of the filter (kernel size = 2 * radius + 1).
 */
void meanSliding(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply a mean (box) filter using a sliding window and return a new image.
 *
 * Convenience wrapper allocating a new output image.
 *
 * @param input Input image view.
 * @param radius Radius of the filter (kernel size = 2 * radius + 1).
 * @return Filtered image.
 */
ImageOwner meanSliding(const ImageView& input, int radius);

/**
 * @brief Stateful sliding window mean filter.
 *
 * This class implements an efficient separable mean filter using
 * horizontal and vertical sliding window passes.
 *
 * Internal buffers are reused across calls, making it suitable for
 * repeated processing (e.g. video streams or pipelines).
 *
 * The implementation includes optimized paths for interior pixels
 * and safe handling of borders.
 */
class MeanSliding
{
public:
    /**
     * @brief Apply the mean filter with a given radius.
     *
     * Performs a horizontal sliding window pass followed by a vertical pass.
     *
     * @param input Input image view.
     * @param radius Radius of the filter (kernel size = 2 * radius + 1).
     *
     * @note Radius is internally clamped to valid image dimensions.
     */
    void apply(const ImageView& input, int radius);

    /**
     * @brief Get a non-owning view of the result image.
     *
     * @return ImageView referencing the internal output buffer.
     *
     * @warning The returned view is valid as long as the object is alive.
     */
    ImageView outputView() const;

    /**
     * @brief Get the result image with ownership.
     *
     * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output() const;

private:
    /**
     * @brief Initialize or resize internal buffers.
     *
     * Ensures that intermediate buffers match the input layout.
     *
     * @param input Input image view.
     */
    void reset(const ImageView& input);

    ImageOwner buffer1_; ///< Intermediate buffer (horizontal pass).
    ImageOwner buffer2_; ///< Intermediate/output buffer (vertical pass).

    int width_{0};  ///< Cached image width.
    int height_{0}; ///< Cached image height.
};

} // namespace fluvel_ip::filter
