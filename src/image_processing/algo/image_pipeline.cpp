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

    if (!params.hasProcessing())
    {
        currentPtr_ = src;
        return;
    }

    if (params.has_gaussian_noise)
    {
        noise::gaussian(src->view(), *dst, params.std_noise);
        std::swap(src, dst);
    }

    if (params.has_salt_noise)
    {
        noise::impulsive(src->view(), *dst, params.proba_noise);
        std::swap(src, dst);
    }

    if (params.has_speckle_noise)
    {
        noise::speckleGamma(src->view(), *dst, params.std_speckle_noise);
        std::swap(src, dst);
    }

    if (params.has_mean_filt)
    {
        filter::mean(src->view(), *dst, params.kernel_mean_length / 2);
        std::swap(src, dst);
    }

    if (params.has_median_filt)
    {
        filter::median(src->view(), *dst, params.kernel_median_length / 2);
        std::swap(src, dst);
    }

    if (params.has_aniso_diff)
    {
        filter::anisotropicDiffusion(src->view(), *dst, params.max_itera, params.lambda,
                                     params.kappa, params.aniso_option);
        std::swap(src, dst);
    }

    if (params.has_open_filt)
    {
        filter::morpho::opening(src->view(), *dst, params.kernel_open_length / 2);
        std::swap(src, dst);
    }

    if (params.has_close_filt)
    {
        filter::morpho::closing(src->view(), *dst, params.kernel_close_length / 2);
        std::swap(src, dst);
    }

    if (params.has_top_hat_filt)
    {
        if (params.is_white_top_hat)
            filter::morpho::topHat(src->view(), *dst, params.kernel_tophat_length / 2);
        else
            filter::morpho::blackTopHat(src->view(), *dst, params.kernel_tophat_length / 2);

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
