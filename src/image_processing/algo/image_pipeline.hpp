// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "anisotropic_diffusion.hpp"
#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

struct ProcessingConfig
{
    static constexpr bool kDefaultProcess = false;

    static constexpr float kDefaultStdNoise = 20.f;
    static constexpr float kDefaultSaltNoise = 0.05f;
    static constexpr float kDefaultSpeckleNoise = 0.16f;

    static constexpr int kDefaultKernelLength = 5;
    static constexpr float kDefaultGaussianSigma = 2.f;

    static constexpr bool kDefault01Algo = true;

    static constexpr filter::ConductionFunction kDefaultAnisoOption =
        filter::ConductionFunction::Exponential;
    static constexpr int kDefaultMaxItera = 10;
    static constexpr double kDefaultLambda = 1.0 / 7.0;
    static constexpr double kDefaultKappa = 30.0;

    static constexpr bool kDefaultWhiteTopHat = true;

    bool enabled = false;

    bool has_gaussian_noise = kDefaultProcess;
    float std_noise = kDefaultStdNoise;

    bool has_salt_noise = kDefaultProcess;
    float proba_noise = kDefaultSaltNoise;

    bool has_speckle_noise = kDefaultProcess;
    float std_speckle_noise = kDefaultSpeckleNoise;

    bool has_median_filt = kDefaultProcess;
    int kernel_median_length = kDefaultKernelLength;
    bool has_O1_algo = kDefault01Algo;

    bool has_mean_filt = kDefaultProcess;
    int kernel_mean_length = kDefaultKernelLength;

    bool has_gaussian_filt = kDefaultProcess;
    int kernel_gaussian_length = kDefaultKernelLength;
    float sigma = kDefaultGaussianSigma;

    bool has_aniso_diff = kDefaultProcess;
    filter::ConductionFunction aniso_option = kDefaultAnisoOption;
    int max_itera = kDefaultMaxItera;
    double lambda = kDefaultLambda;
    double kappa = kDefaultKappa;

    bool has_open_filt = kDefaultProcess;
    int kernel_open_length = kDefaultKernelLength;

    bool has_close_filt = kDefaultProcess;
    int kernel_close_length = kDefaultKernelLength;

    bool has_top_hat_filt = kDefaultProcess;
    bool is_white_top_hat = kDefaultWhiteTopHat;
    int kernel_tophat_length = kDefaultKernelLength;

    bool has_O1_morpho = kDefault01Algo;

    bool hasProcessing() const
    {
        return enabled && (has_gaussian_noise || has_salt_noise || has_speckle_noise ||
                           has_median_filt || has_mean_filt || has_gaussian_filt ||
                           has_aniso_diff || has_open_filt || has_close_filt || has_top_hat_filt);
    }
};

class ImagePipeline
{
public:
    void reset(const ImageView& input);
    void apply(const ImageView& input, const ProcessingConfig& config);

    ImageView outputView() const;
    const ImageOwner& output() const;

private:
    ImageOwner bufferA_;
    ImageOwner bufferB_;

    ImageOwner* currentPtr_ = nullptr;
};

} // namespace fluvel_ip
