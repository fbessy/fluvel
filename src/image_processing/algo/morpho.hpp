// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter::morpho
{

/**
 * @brief Apply a morphological dilation (max filter).
 *
 * Computes the maximum value in a square structuring element of radius @p radius.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param radius Radius of the structuring element (kernel size = 2 * radius + 1).
 */
void max(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply a morphological dilation (max filter) and return a new image.
 */
ImageOwner max(const ImageView& input, int radius);

/**
 * @brief Apply a morphological erosion (min filter).
 *
 * Computes the minimum value in a square structuring element of radius @p radius.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param radius Radius of the structuring element.
 */
void min(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply a morphological erosion (min filter) and return a new image.
 */
ImageOwner min(const ImageView& input, int radius);

/**
 * @brief Apply morphological opening (erosion followed by dilation).
 *
 * Useful for removing small bright structures (noise) while preserving shape.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param radius Radius of the structuring element.
 */
void opening(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply morphological opening and return a new image.
 */
ImageOwner opening(const ImageView& input, int radius);

/**
 * @brief Apply morphological closing (dilation followed by erosion).
 *
 * Useful for filling small dark holes and gaps.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param radius Radius of the structuring element.
 */
void closing(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply morphological closing and return a new image.
 */
ImageOwner closing(const ImageView& input, int radius);

/**
 * @brief Apply white top-hat transform.
 *
 * Computes the difference between the input image and its opening:
 * result = input - opening(input).
 *
 * Highlights small bright features.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param radius Radius of the structuring element.
 */
void topHat(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply white top-hat transform and return a new image.
 */
ImageOwner topHat(const ImageView& input, int radius);

/**
 * @brief Apply black top-hat transform.
 *
 * Computes the difference between the closing and the input:
 * result = closing(input) - input.
 *
 * Highlights small dark features.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param radius Radius of the structuring element.
 */
void blackTopHat(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply black top-hat transform and return a new image.
 */
ImageOwner blackTopHat(const ImageView& input, int radius);

/**
 * @brief Apply morphological gradient.
 *
 * Computes the difference between dilation and erosion:
 * result = max(input) - min(input).
 *
 * Highlights edges and transitions.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param radius Radius of the structuring element.
 */
void gradient(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply morphological gradient and return a new image.
 */
ImageOwner gradient(const ImageView& input, int radius);

} // namespace fluvel_ip::filter::morpho
