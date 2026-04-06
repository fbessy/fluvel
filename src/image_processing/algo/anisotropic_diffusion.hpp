// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

#include <vector>

namespace fluvel_ip::filter
{

enum class ConductionFunction
{
    Exponential, ///< exp(-(∇I/kappa)^2)
    Rational     ///< 1 / (1 + (∇I/kappa)^2)
};

inline constexpr ConductionFunction kDefaultConductionFunction = ConductionFunction::Exponential;
inline constexpr int kDefaultIterations = 10;
inline constexpr double kDefaultLambda = 1.0 / 7.0;
inline constexpr double kDefaultKappa = 30.0;

/**
 * @brief Apply Perona-Malik anisotropic diffusion.
 *
 * Stateless version: allocates internal buffers.
 * For real-time usage, prefer the AnisotropicDiffusion class.
 */
ImageOwner anisotropicDiffusion(const ImageView& input, int iterations = kDefaultIterations,
                                double lambda = kDefaultLambda, double kappa = kDefaultKappa,
                                ConductionFunction conduction = kDefaultConductionFunction);

class AnisotropicDiffusion
{
public:
    void reset(const ImageView& input);

    void apply(int iterations = kDefaultIterations, double lambda = kDefaultLambda,
               double kappa = kDefaultKappa,
               ConductionFunction conduction = kDefaultConductionFunction);

    ImageView outputView() const;
    const ImageOwner& output() const;

private:
    int w_ = 0;
    int h_ = 0;
    int channels_ = 0;
    int activeChannels_ = 0;
    int stridePad_ = 0;

    std::vector<double> current_;
    std::vector<double> next_;

    ImageOwner output_;

    int idx(int x, int y, int c) const
    {
        return ((x + 1) + (y + 1) * stridePad_) * activeChannels_ + c;
    }

    void initFromInput(const ImageView& input);
    void padBorders();
};

} // namespace fluvel_ip::filter
