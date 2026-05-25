// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "noise.hpp"

#include <algorithm>
#include <cstdint>

namespace fluvel_ip::noise
{

ImageOwner gaussian(const ImageView& input, float sigma)
{
    ImageOwner out(input.width(), input.height(), input.format());
    gaussian(input, out, sigma);
    return out;
}

void gaussian(const ImageView& input, ImageOwner& output, float sigma)
{
    output.ensureLike(input);

    constexpr float kSigmaMin = 1e-3f;
    constexpr float kSigmaMax = 80.f;

    sigma = std::clamp(sigma, kSigmaMin, kSigmaMax);

    std::normal_distribution<float> dist(0.f, sigma);

    const int w = input.width();
    const int h = input.height();
    const int channels = input.channels();
    const int activeChannels = std::min(channels, 3);

    auto& rng = noise::rng();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = output.rowPtr(y);

        for (int x = 0; x < w; ++x)
        {
            const int idx = x * channels;

            for (int c = 0; c < activeChannels; ++c)
            {
                float v = src[idx + c] + dist(rng);
                v = std::clamp(v, 0.f, 255.f);

                dst[idx + c] = static_cast<uint8_t>(v + 0.5f);
            }

            // preserve alpha
            if (channels == 4)
                dst[idx + 3] = src[idx + 3];
        }
    }
}

ImageOwner impulsive(const ImageView& input, float probability)
{
    ImageOwner out(input.width(), input.height(), input.format());
    impulsive(input, out, probability);
    return out;
}

void impulsive(const ImageView& input, ImageOwner& output, float probability)
{
    output.ensureLike(input);

    probability = std::clamp(probability, 0.f, 1.f);

    std::uniform_real_distribution<float> dist01(0.f, 1.f);
    std::uniform_int_distribution<int> distSaltPepper(0, 1);

    const int w = input.width();
    const int h = input.height();
    const int channels = input.channels();
    const int activeChannels = std::min(channels, 3);

    auto& rng = noise::rng();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = output.rowPtr(y);

        for (int x = 0; x < w; ++x)
        {
            if (channels == 1)
            {
                // ===== Grayscale : salt & pepper =====
                if (dist01(rng) < probability)
                    dst[x] = distSaltPepper(rng) ? 255 : 0;
                else
                    dst[x] = src[x];
            }
            else
            {
                const int idx = x * channels;

                // ===== Couleur : impulsif par canal =====
                for (int c = 0; c < activeChannels; ++c)
                {
                    if (dist01(rng) < probability)
                        dst[idx + c] = distSaltPepper(rng) ? 255 : 0;
                    else
                        dst[idx + c] = src[idx + c];
                }

                // preserve alpha
                if (channels == 4)
                    dst[idx + 3] = src[idx + 3];
            }
        }
    }
}

ImageOwner speckleUniform(const ImageView& input, float sigma)
{
    ImageOwner out(input.width(), input.height(), input.format());
    speckleUniform(input, out, sigma);
    return out;
}

void speckleUniform(const ImageView& input, ImageOwner& output, float sigma)
{
    output.ensureLike(input);

    constexpr float kSigmaMin = 1e-3f;
    constexpr float kSigmaMax = 80.f;

    sigma = std::clamp(sigma, kSigmaMin, kSigmaMax);

    std::normal_distribution<float> dist(0.f, sigma);

    const int w = input.width();
    const int h = input.height();
    const int channels = input.channels();
    const int activeChannels = std::min(channels, 3);

    auto& rng = noise::rng();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = output.rowPtr(y);

        for (int x = 0; x < w; ++x)
        {
            const int idx = x * channels;

            // multiplicative noise factor
            float noise = 1.f + dist(rng);

            for (int c = 0; c < activeChannels; ++c)
            {
                float v = src[idx + c] * noise;
                v = std::clamp(v, 0.f, 255.f);

                dst[idx + c] = static_cast<uint8_t>(v + 0.5f);
            }

            // preserve alpha
            if (channels == 4)
                dst[idx + 3] = src[idx + 3];
        }
    }
}

ImageOwner speckleGamma(const ImageView& input, float sigma)
{
    ImageOwner out(input.width(), input.height(), input.format());
    speckleGamma(input, out, sigma);
    return out;
}

void speckleGamma(const ImageView& input, ImageOwner& output, float sigma)
{
    output.ensureLike(input);

    constexpr float kSigmaMin = 1e-3f;
    constexpr float kSigmaMax = 2.f;

    sigma = std::clamp(sigma, kSigmaMin, kSigmaMax);

    // Mapping sigma -> L (nombre de looks)
    const float L = 1.f / (sigma * sigma);

    // Gamma(L, 1/L) → mean = 1
    std::gamma_distribution<float> dist(L, 1.f / L);

    const int w = input.width();
    const int h = input.height();
    const int channels = input.channels();
    const int activeChannels = std::min(channels, 3);

    auto& rng = noise::rng();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = output.rowPtr(y);

        for (int x = 0; x < w; ++x)
        {
            const int idx = x * channels;

            for (int c = 0; c < activeChannels; ++c)
            {
                float n = dist(rng); // multiplicatif (mean = 1)

                float v = src[idx + c] * n;

                v = std::clamp(v, 0.f, 255.f);
                dst[idx + c] = static_cast<uint8_t>(v + 0.5f);
            }

            // Preserve alpha
            if (channels == 4)
                dst[idx + 3] = src[idx + 3];
        }
    }
}

} // namespace fluvel_ip::noise
