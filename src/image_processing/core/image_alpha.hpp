// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

/**
 * @brief Copy alpha channel from input to output.
 *
 * If the image format does not contain an alpha channel,
 * the function does nothing.
 *
 * @param input Source image.
 * @param output Destination image.
 */
void copyAlpha(const ImageView& input, ImageOwner& output);

/**
 * @brief Fill alpha channel with a constant value.
 *
 * If the image format does not contain an alpha channel,
 * the function does nothing.
 *
 * @param image Image to modify.
 * @param alpha Alpha value to assign.
 */
void fillAlpha(ImageOwner& image, uint8_t value = 255);

} // namespace fluvel_ip