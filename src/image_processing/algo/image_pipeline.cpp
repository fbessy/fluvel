// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_pipeline.hpp"
#include "anisotropic_diffusion.hpp"
#include "mean_filter.hpp"
#include "median_filter.hpp"
#include "morpho.hpp"
#include "noise.hpp"

namespace fluvel_ip
{

void ImagePipeline::reset(const ImageView& input)
{
    // Allouer si nécessaire
    if (!bufferA_.hasSameLayout(input))
    {
        bufferA_ = ImageOwner::like(input);
        bufferB_ = ImageOwner::like(input);
    }
}

void ImagePipeline::apply(const ImageView& input, const ProcessingParams& params)
{
    reset(input);

    bufferA_.copyFrom(input);

    ImageOwner* src = &bufferA_;
    ImageOwner* dst = &bufferB_;

    if (!params.hasActiveProcessing())
    {
        currentPtr_ = src;
        return;
    }

    if (params.gaussianNoiseEnabled)
    {
        noise::gaussian(src->view(), *dst, params.noiseStdDev);
        std::swap(src, dst);
    }

    if (params.saltNoiseEnabled)
    {
        noise::impulsive(src->view(), *dst, params.saltNoiseProbability);
        std::swap(src, dst);
    }

    if (params.speckleNoiseEnabled)
    {
        noise::speckleGamma(src->view(), *dst, params.speckleNoiseStdDev);
        std::swap(src, dst);
    }

    if (params.meanFilterEnabled)
    {
        filter::mean(src->view(), *dst, params.meanKernelSize / 2);
        std::swap(src, dst);
    }

    if (params.medianFilterEnabled)
    {
        filter::median(src->view(), *dst, params.medianKernelSize / 2);
        std::swap(src, dst);
    }

    if (params.anisotropicDiffusionEnabled)
    {
        filter::anisotropicDiffusion(src->view(), *dst, params.aniso);
        std::swap(src, dst);
    }

    if (params.openingEnabled)
    {
        filter::morpho::opening(src->view(), *dst, params.openingKernelSize / 2);
        std::swap(src, dst);
    }

    if (params.closingEnabled)
    {
        filter::morpho::closing(src->view(), *dst, params.closingKernelSize / 2);
        std::swap(src, dst);
    }

    if (params.topHatEnabled)
    {
        if (params.useWhiteTopHat)
            filter::morpho::topHat(src->view(), *dst, params.topHatKernelSize / 2);
        else
            filter::morpho::blackTopHat(src->view(), *dst, params.topHatKernelSize / 2);

        std::swap(src, dst);
    }

    currentPtr_ = src;
}

ImageView ImagePipeline::outputView() const
{
    assert(currentPtr_);

    return currentPtr_->view();
}

const ImageOwner& ImagePipeline::output() const
{
    assert(currentPtr_);

    return *currentPtr_;
}

} // namespace fluvel_ip
