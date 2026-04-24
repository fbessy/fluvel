// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter
{

/**
 * @brief Apply a mean (box) filter with a preallocated output buffer.
 *
 * This function applies a mean filter of radius @p radius to the input image.
 * The implementation may internally select an optimized strategy depending
 * on the radius (e.g. specialized 3x3, sliding window).
 *
 * The result is written into @p output.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param radius Radius of the filter (kernel size = 2 * radius + 1).
 *
 * @note Radius is internally clamped to valid image dimensions.
 */
void mean(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply a mean (box) filter and return a new image.
 *
 * Convenience wrapper allocating a new output image.
 *
 * @param input Input image view.
 * @param radius Radius of the filter (kernel size = 2 * radius + 1).
 * @return Filtered image.
 */
ImageOwner mean(const ImageView& input, int radius);

} // namespace fluvel_ip::filter
