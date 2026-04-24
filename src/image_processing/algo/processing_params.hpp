// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "anisotropic_diffusion.hpp"

namespace fluvel_ip
{

struct ProcessingParams
{
    static constexpr bool kDefaultDisabled = false;

    static constexpr float kDefaultStdNoise = 20.f;
    static constexpr float kDefaultSaltNoise = 0.05f;
    static constexpr float kDefaultSpeckleNoise = 0.16f;

    static constexpr int kDefaultKernelSize = 5;

    static constexpr filter::ConductionFunction kDefaultConductionFunction =
        filter::ConductionFunction::Exponential;
    static constexpr int kDefaultMaxIterations = 10;
    static constexpr double kDefaultLambda = 1.0 / 7.0;
    static constexpr double kDefaultKappa = 30.0;

    static constexpr bool kDefaultWhiteTopHat = true;

    bool processingEnabled = kDefaultDisabled;

    bool gaussianNoiseEnabled = kDefaultDisabled;
    float noiseStdDev = kDefaultStdNoise;

    bool saltNoiseEnabled = kDefaultDisabled;
    float saltNoiseProbability = kDefaultSaltNoise;

    bool speckleNoiseEnabled = kDefaultDisabled;
    float speckleNoiseStdDev = kDefaultSpeckleNoise;

    bool medianFilterEnabled = kDefaultDisabled;
    int medianKernelSize = kDefaultKernelSize;

    bool meanFilterEnabled = kDefaultDisabled;
    int meanKernelSize = kDefaultKernelSize;

    bool anisotropicDiffusionEnabled = kDefaultDisabled;
    filter::ConductionFunction conductionFunction = kDefaultConductionFunction;
    int maxIterations = kDefaultMaxIterations;
    double lambda = kDefaultLambda;
    double kappa = kDefaultKappa;

    bool openingEnabled = kDefaultDisabled;
    int openingKernelSize = kDefaultKernelSize;

    bool closingEnabled = kDefaultDisabled;
    int closingKernelSize = kDefaultKernelSize;

    bool topHatEnabled = kDefaultDisabled;
    bool useWhiteTopHat = kDefaultWhiteTopHat;
    int topHatKernelSize = kDefaultKernelSize;

    bool hasActiveProcessing() const noexcept
    {
        return processingEnabled &&
               (gaussianNoiseEnabled || saltNoiseEnabled || speckleNoiseEnabled ||
                medianFilterEnabled || meanFilterEnabled || anisotropicDiffusionEnabled ||
                openingEnabled || closingEnabled || topHatEnabled);
    }
};

} // namespace fluvel_ip
