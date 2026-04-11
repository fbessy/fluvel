// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_pipeline.hpp"
#include "anisotropic_diffusion.hpp"
#include "mean_filter.hpp"
#include "noise.hpp"

namespace fluvel_ip
{

void ImagePipeline::reset(const ImageView& input)
{
    // Allouer si nécessaire
    if (bufferA_.width() != input.width() || bufferA_.height() != input.height() ||
        bufferA_.format() != input.format())
    {
        bufferA_ = ImageOwner(input.width(), input.height(), input.format());
        bufferB_ = ImageOwner(input.width(), input.height(), input.format());
    }
}

void ImagePipeline::apply(const ImageView& input, const ProcessingConfig& config)
{
    reset(input);

    bufferA_.copyFrom(input);

    ImageOwner* src = &bufferA_;
    ImageOwner* dst = &bufferB_;

    if (!config.hasProcessing())
    {
        currentPtr_ = src;
        return;
    }

    if (config.has_gaussian_noise)
    {
        noise::gaussian(src->view(), *dst, config.std_noise);
        std::swap(src, dst);
    }

    if (config.has_salt_noise)
    {
        noise::impulsive(src->view(), *dst, config.proba_noise);
        std::swap(src, dst);
    }

    if (config.has_speckle_noise)
    {
        noise::speckleGamma(src->view(), *dst, config.std_speckle_noise);
        std::swap(src, dst);
    }

    if (config.has_mean_filt)
    {
        filter::mean(src->view(), *dst, config.kernel_mean_length / 2);
        std::swap(src, dst);
    }

    if (config.has_aniso_diff)
    {
        filter::anisotropicDiffusion(src->view(), *dst, config.max_itera, config.lambda,
                                     config.kappa, config.aniso_option);
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
