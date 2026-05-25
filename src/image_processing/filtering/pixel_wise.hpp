// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::pixelwise
{

/**
 * @brief Compute pixel-wise saturated difference between two images.
 *
 * For each pixel and channel, computes:
 *
 *   out = max(0, a - b)
 *
 * Both input images must have identical dimensions and format.
 *
 * @param a First input image.
 * @param b Second input image.
 * @param out Output image (must match input layout or be reallocated accordingly).
 *
 * @note Common uses include:
 * - morphological white top-hat transforms
 * - morphological gradients
 * - directional image comparison
 */
void subtract(const ImageView& a, const ImageView& b, ImageOwner& out);

} // namespace fluvel_ip::pixelwise
