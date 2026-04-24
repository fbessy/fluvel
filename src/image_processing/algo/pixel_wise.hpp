// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter::pixelwise
{

/**
 * @brief Compute pixel-wise absolute difference between two images.
 *
 * For each pixel and channel, computes:
 *   out = |a - b|
 *
 * Both input images must have identical dimensions and format.
 *
 * @param a First input image.
 * @param b Second input image.
 * @param out Output image (must match input layout or be reallocated accordingly).
 *
 * @note This function is commonly used for:
 * - debugging image processing pipelines
 * - visualizing differences between filters
 * - motion detection or change detection
 */
void diff(const ImageView& a, const ImageView& b, ImageOwner& out);

} // namespace fluvel_ip::filter::pixelwise
