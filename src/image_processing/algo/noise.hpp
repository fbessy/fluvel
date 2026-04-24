// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

/**
 * @brief Default standard deviation for additive Gaussian noise.
 */
static constexpr float kDefaultGaussianSigmaNoise = 20.f;

/**
 * @brief Default probability for impulsive (salt-and-pepper) noise.
 */
static constexpr float kDefaultImpulsiveNoise = 0.05f;

/**
 * @brief Default standard deviation for multiplicative speckle noise.
 */
static constexpr float kDefaultSpeckleNoise = 0.16f;

namespace fluvel_ip::noise
{

/**
 * @brief Apply additive Gaussian noise.
 *
 * Adds zero-mean Gaussian noise with standard deviation @p sigma to the image.
 *
 * @param input Input image view.
 * @param sigma Standard deviation of the noise.
 * @return Noisy image.
 */
ImageOwner gaussian(const ImageView& input, float sigma = kDefaultGaussianSigmaNoise);

/**
 * @brief Apply additive Gaussian noise using a preallocated output buffer.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param sigma Standard deviation of the noise.
 */
void gaussian(const ImageView& input, ImageOwner& output, float sigma = kDefaultGaussianSigmaNoise);

/**
 * @brief Apply impulsive (salt-and-pepper) noise.
 *
 * Randomly replaces pixels with extreme values (typically 0 or 255)
 * with probability @p probability.
 *
 * @param input Input image view.
 * @param probability Probability of corruption per pixel.
 * @return Noisy image.
 */
ImageOwner impulsive(const ImageView& input, float probability = kDefaultImpulsiveNoise);

/**
 * @brief Apply impulsive (salt-and-pepper) noise using a preallocated buffer.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param probability Probability of corruption per pixel.
 */
void impulsive(const ImageView& input, ImageOwner& output,
               float probability = kDefaultImpulsiveNoise);

/**
 * @brief Apply multiplicative speckle noise (uniform distribution).
 *
 * Applies noise of the form:
 *   output = input * (1 + noise)
 * where noise is uniformly distributed.
 *
 * @param input Input image view.
 * @param sigma Noise strength.
 * @return Noisy image.
 */
ImageOwner speckleUniform(const ImageView& input, float sigma = kDefaultSpeckleNoise);

/**
 * @brief Apply multiplicative speckle noise (uniform distribution) using a preallocated buffer.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param sigma Noise strength.
 */
void speckleUniform(const ImageView& input, ImageOwner& output, float sigma = kDefaultSpeckleNoise);

/**
 * @brief Apply multiplicative speckle noise (gamma distribution).
 *
 * Applies noise of the form:
 *   output = input * noise
 * where noise follows a gamma distribution.
 *
 * Commonly used to simulate radar or ultrasound noise.
 *
 * @param input Input image view.
 * @param sigma Noise parameter controlling variance.
 * @return Noisy image.
 */
ImageOwner speckleGamma(const ImageView& input, float sigma = kDefaultSpeckleNoise);

/**
 * @brief Apply multiplicative speckle noise (gamma distribution) using a preallocated buffer.
 *
 * @param input Input image view.
 * @param output Output image owner.
 * @param sigma Noise parameter.
 */
void speckleGamma(const ImageView& input, ImageOwner& output, float sigma = kDefaultSpeckleNoise);

} // namespace fluvel_ip::noise
