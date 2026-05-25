// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "anisotropic_diffusion.hpp"

namespace fluvel_ip
{

/**
 * @brief Parameters controlling the image processing pipeline.
 *
 * This structure groups all configurable options for preprocessing
 * operations applied to an image.
 *
 * It includes:
 * - Noise generation
 * - Linear and non-linear filtering
 * - Anisotropic diffusion
 * - Morphological operations
 *
 * Each operation can be enabled independently.
 */
struct ProcessingParams
{
    static constexpr bool kDefaultDisabled{false};

    static constexpr float kDefaultStdNoise{20.f};
    static constexpr float kDefaultSaltNoise{0.05f};
    static constexpr float kDefaultSpeckleNoise{0.16f};

    static constexpr int kDefaultKernelSize{5};

    static constexpr bool kDefaultWhiteTopHat{true};

    /**
     * @brief Global flag enabling or disabling all processing.
     */
    bool processingEnabled{kDefaultDisabled};

    // =========================
    // Noise
    // =========================

    /// Enable additive Gaussian noise.
    bool gaussianNoiseEnabled{kDefaultDisabled};

    /// Standard deviation of Gaussian noise.
    float noiseStdDev{kDefaultStdNoise};

    /// Enable impulsive (salt-and-pepper) noise.
    bool saltNoiseEnabled{kDefaultDisabled};

    /// Probability of salt-and-pepper noise.
    float saltNoiseProbability{kDefaultSaltNoise};

    /// Enable multiplicative speckle noise.
    bool speckleNoiseEnabled{kDefaultDisabled};

    /// Standard deviation of speckle noise.
    float speckleNoiseStdDev{kDefaultSpeckleNoise};

    // =========================
    // Filtering
    // =========================

    /// Enable median filter.
    bool medianFilterEnabled{kDefaultDisabled};

    /// Kernel size (radius-based) for median filter.
    int medianKernelSize{kDefaultKernelSize};

    /// Enable mean (box) filter.
    bool meanFilterEnabled{kDefaultDisabled};

    /// Kernel size (radius-based) for mean filter.
    int meanKernelSize{kDefaultKernelSize};

    // =========================
    // Anisotropic diffusion
    // =========================

    /// Enable anisotropic diffusion.
    bool anisotropicDiffusionEnabled{kDefaultDisabled};

    /// Anistropic diffusion parameters.
    filter::AnisoParams aniso{};

    // =========================
    // Morphological operations
    // =========================

    /// Enable morphological opening.
    bool openingEnabled{kDefaultDisabled};

    /// Kernel size for opening.
    int openingKernelSize{kDefaultKernelSize};

    /// Enable morphological closing.
    bool closingEnabled{kDefaultDisabled};

    /// Kernel size for closing.
    int closingKernelSize{kDefaultKernelSize};

    /// Enable top-hat transform.
    bool topHatEnabled{kDefaultDisabled};

    /// Select white (true) or black (false) top-hat.
    bool useWhiteTopHat{kDefaultWhiteTopHat};

    /// Kernel size for top-hat.
    int topHatKernelSize{kDefaultKernelSize};

    /**
     * @brief Check if any processing step is active.
     *
     * @return true if processing is enabled and at least one operation is active.
     */
    bool hasActiveProcessing() const noexcept
    {
        return processingEnabled &&
               (gaussianNoiseEnabled || saltNoiseEnabled || speckleNoiseEnabled ||
                medianFilterEnabled || meanFilterEnabled || anisotropicDiffusionEnabled ||
                openingEnabled || closingEnabled || topHatEnabled);
    }
};

} // namespace fluvel_ip
