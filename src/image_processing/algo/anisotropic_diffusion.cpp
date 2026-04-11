// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "anisotropic_diffusion.hpp"

#include <algorithm>
#include <utility>

namespace fluvel_ip::filter
{

void anisotropicDiffusion(const ImageView& input, ImageOwner& output, int iterations, double lambda,
                          double kappa, ConductionFunction conduction)
{
    AnisotropicDiffusion impl;

    impl.reset(input);
    impl.apply(iterations, lambda, kappa, conduction);

    std::swap(output, impl.outputRef());
}

ImageOwner anisotropicDiffusion(const ImageView& input, int iterations, double lambda, double kappa,
                                ConductionFunction conduction)
{
    AnisotropicDiffusion impl;

    impl.reset(input);
    impl.apply(iterations, lambda, kappa, conduction);

    return impl.output(); // copy (safe)
}

void AnisotropicDiffusion::reset(const ImageView& input)
{
    w_ = input.width();
    h_ = input.height();
    channels_ = input.channels();
    activeChannels_ = std::min(channels_, 3);
    stridePad_ = w_ + 2;

    current_.resize((w_ + 2) * (h_ + 2) * activeChannels_);
    next_.resize((w_ + 2) * (h_ + 2) * activeChannels_);

    output_ = ImageOwner::like(input);

    initFromInput(input);
    padBorders();
}

void AnisotropicDiffusion::initFromInput(const ImageView& input)
{
    for (int y = 0; y < h_; ++y)
    {
        const unsigned char* row = input.row(y);

        for (int x = 0; x < w_; ++x)
        {
            for (int c = 0; c < activeChannels_; ++c)
            {
                current_[idx(x, y, c)] = row[x * channels_ + c];
            }
        }
    }
}

void AnisotropicDiffusion::padBorders()
{
    // --- LEFT & RIGHT columns
    for (int y = 0; y < h_; ++y)
    {
        for (int c = 0; c < activeChannels_; ++c)
        {
            // x = -1 (left)
            current_[idx(-1, y, c)] = current_[idx(0, y, c)];

            // x = w (right)
            current_[idx(w_, y, c)] = current_[idx(w_ - 1, y, c)];
        }
    }

    // --- TOP & BOTTOM rows
    for (int x = 0; x < w_; ++x)
    {
        for (int c = 0; c < activeChannels_; ++c)
        {
            // y = -1 (top)
            current_[idx(x, -1, c)] = current_[idx(x, 0, c)];

            // y = h (bottom)
            current_[idx(x, h_, c)] = current_[idx(x, h_ - 1, c)];
        }
    }

    // --- CORNERS
    for (int c = 0; c < activeChannels_; ++c)
    {
        current_[idx(-1, -1, c)] = current_[idx(0, 0, c)];
        current_[idx(w_, -1, c)] = current_[idx(w_ - 1, 0, c)];
        current_[idx(-1, h_, c)] = current_[idx(0, h_ - 1, c)];
        current_[idx(w_, h_, c)] = current_[idx(w_ - 1, h_ - 1, c)];
    }
}

void AnisotropicDiffusion::apply(int iterations, double lambda, double kappa,
                                 ConductionFunction conduction)
{
    const double invKappa2 = 1.0 / (kappa * kappa);
    const double dist[8] = {2.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 2.0};
    const bool useExp = (conduction == ConductionFunction::Exponential);

    for (int it = 0; it < iterations; ++it)
    {
        for (int y = 0; y < h_; ++y)
        {
            for (int x = 0; x < w_; ++x)
            {
                for (int c = 0; c < activeChannels_; ++c)
                {
                    int p = idx(x, y, c);
                    double center = current_[p];

                    // gradients (no bounds check thanks to padding)
                    double nabla[8] = {current_[idx(x - 1, y - 1, c)] - center,
                                       current_[idx(x, y - 1, c)] - center,
                                       current_[idx(x + 1, y - 1, c)] - center,
                                       current_[idx(x - 1, y, c)] - center,
                                       current_[idx(x + 1, y, c)] - center,
                                       current_[idx(x - 1, y + 1, c)] - center,
                                       current_[idx(x, y + 1, c)] - center,
                                       current_[idx(x + 1, y + 1, c)] - center};

                    double sigma = 0.0;

                    for (int k = 0; k < 8; ++k)
                    {
                        double val = nabla[k] * nabla[k] * invKappa2;

                        double cond = useExp ? std::exp(-val) : 1.0 / (1.0 + val);

                        sigma += (cond * nabla[k]) / dist[k];
                    }

                    next_[p] = center + lambda * sigma;
                }
            }
        }

        std::swap(current_, next_);

        // OPTIONNEL (plus rigoureux si diffusion touche les bords)
        padBorders();
    }

    // =========================
    // Convert to output
    // =========================
    for (int y = 0; y < h_; ++y)
    {
        unsigned char* dst = output_.data() + y * output_.stride();

        for (int x = 0; x < w_; ++x)
        {
            for (int c = 0; c < activeChannels_; ++c)
            {
                double v = current_[idx(x, y, c)];
                v = std::clamp(v, 0.0, 255.0);

                dst[x * channels_ + c] = static_cast<unsigned char>(v + 0.5);
            }

            // preserve alpha
            if (channels_ == 4)
            {
                dst[x * 4 + 3] = 255;
            }
        }
    }
}

ImageView AnisotropicDiffusion::outputView() const
{
    return output_.view();
}

const ImageOwner& AnisotropicDiffusion::output() const
{
    return output_;
}

ImageOwner& AnisotropicDiffusion::outputRef()
{
    return output_;
}

} // namespace fluvel_ip
