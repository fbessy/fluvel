// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

static constexpr float kDefaultGaussianSigmaNoise = 20.f;
static constexpr float kDefaultImpulsiveNoise = 0.05f;
static constexpr float kDefaultSpeckleNoise = 0.16f;

namespace fluvel_ip::noise
{

ImageOwner gaussian(const ImageView& input, float sigma = kDefaultGaussianSigmaNoise);
void gaussian(const ImageView& input, ImageOwner& output, float sigma = kDefaultGaussianSigmaNoise);

ImageOwner impulsive(const ImageView& input, float probability = kDefaultImpulsiveNoise);
void impulsive(const ImageView& input, ImageOwner& output,
               float probability = kDefaultImpulsiveNoise);

ImageOwner speckleUniform(const ImageView& input, float sigma = kDefaultSpeckleNoise);
void speckleUniform(const ImageView& input, ImageOwner& output, float sigma = kDefaultSpeckleNoise);

ImageOwner speckleGamma(const ImageView& input, float sigma = kDefaultSpeckleNoise);
void speckleGamma(const ImageView& input, ImageOwner& output, float sigma = kDefaultSpeckleNoise);

} // namespace fluvel_ip::noise
