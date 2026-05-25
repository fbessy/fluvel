// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "mean_filter_sliding.hpp"

#include <algorithm>
#include <cassert>

namespace fluvel_ip::filter
{

void meanSliding(const ImageView& input, ImageOwner& output, int radius)
{
    MeanSliding filter;
    filter.apply(input, radius);
    output.copyFrom(filter.outputView());
}

ImageOwner meanSliding(const ImageView& input, int radius)
{
    MeanSliding filter;
    filter.apply(input, radius);
    return filter.output(); // copy (safe)
}

void MeanSliding::reset(const ImageView& input)
{
    if (!buffer1_.hasSameLayout(input))
    {
        buffer1_ = ImageOwner::like(input);
        buffer2_ = ImageOwner::like(input);
    }

    width_ = input.width();
    height_ = input.height();
}

void MeanSliding::apply(const ImageView& input, int radius)
{
    assert(radius >= 1);

    reset(input);

    const int maxValidRadius = std::min(width_ - 1, height_ - 1);

    // clamp radius to avoid out-of-bounds when kernel exceeds image size
    radius = std::min(radius, maxValidRadius);

    const int channels = input.channels();
    const int kernelSize = 2 * radius + 1;

    // =========================
    // PASS 1 — Horizontal (sliding + fast path)
    // =========================
    for (int y = 0; y < height_; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = buffer1_.rowPtr(y);

        for (int c = 0; c < channels; ++c)
        {
            int sum = 0;

            // init window (avec clamp uniquement ici)
            for (int k = -radius; k <= radius; ++k)
            {
                int xx = std::clamp(k, 0, width_ - 1);
                sum += src[xx * channels + c];
            }

            // -------- LEFT BORDER --------
            for (int x = 0; x < radius; ++x)
            {
                int idx = x * channels + c;
                dst[idx] = static_cast<uint8_t>((sum + kernelSize / 2) / kernelSize);

                int x_out = std::clamp(x - radius, 0, width_ - 1);
                int x_in = std::clamp(x + radius + 1, 0, width_ - 1);

                sum -= src[x_out * channels + c];
                sum += src[x_in * channels + c];
            }

            // -------- CENTER (FAST PATH) --------
            for (int x = radius; x < width_ - radius - 1; ++x)
            {
                int idx = x * channels + c;
                dst[idx] = static_cast<uint8_t>((sum + kernelSize / 2) / kernelSize);

                int x_out = x - radius;
                int x_in = x + radius + 1;

                sum -= src[x_out * channels + c];
                sum += src[x_in * channels + c];
            }

            // -------- RIGHT BORDER --------
            for (int x = width_ - radius - 1; x < width_; ++x)
            {
                int idx = x * channels + c;
                dst[idx] = static_cast<uint8_t>((sum + kernelSize / 2) / kernelSize);

                int x_out = std::clamp(x - radius, 0, width_ - 1);
                int x_in = std::clamp(x + radius + 1, 0, width_ - 1);

                sum -= src[x_out * channels + c];
                sum += src[x_in * channels + c];
            }
        }

        // alpha à 255 si besoin
        if (channels == 4)
        {
            for (int x = 0; x < width_; ++x)
                dst[x * channels + 3] = 255;
        }
    }

    // =========================
    // PASS 2 — Vertical (sliding + fast path)
    // =========================
    for (int x = 0; x < width_; ++x)
    {
        for (int c = 0; c < channels; ++c)
        {
            int sum = 0;

            // init window
            for (int k = -radius; k <= radius; ++k)
            {
                int yy = std::clamp(k, 0, height_ - 1);
                const uint8_t* row = buffer1_.rowPtr(yy);

                sum += row[x * channels + c];
            }

            // -------- TOP BORDER --------
            for (int y = 0; y < radius; ++y)
            {
                uint8_t* dst = buffer2_.rowPtr(y);

                int idx = x * channels + c;
                dst[idx] = static_cast<uint8_t>((sum + kernelSize / 2) / kernelSize);

                int y_out = std::clamp(y - radius, 0, height_ - 1);
                int y_in = std::clamp(y + radius + 1, 0, height_ - 1);

                const uint8_t* row_out = buffer1_.rowPtr(y_out);
                const uint8_t* row_in = buffer1_.rowPtr(y_in);

                sum -= row_out[x * channels + c];
                sum += row_in[x * channels + c];
            }

            // -------- CENTER (FAST PATH) --------
            for (int y = radius; y < height_ - radius - 1; ++y)
            {
                uint8_t* dst = buffer2_.rowPtr(y);

                int idx = x * channels + c;
                dst[idx] = static_cast<uint8_t>((sum + kernelSize / 2) / kernelSize);

                int y_out = y - radius;
                int y_in = y + radius + 1;

                const uint8_t* row_out = buffer1_.rowPtr(y_out);
                const uint8_t* row_in = buffer1_.rowPtr(y_in);

                sum -= row_out[x * channels + c];
                sum += row_in[x * channels + c];
            }

            // -------- BOTTOM BORDER --------
            for (int y = height_ - radius - 1; y < height_; ++y)
            {
                uint8_t* dst = buffer2_.rowPtr(y);

                int idx = x * channels + c;
                dst[idx] = static_cast<uint8_t>((sum + kernelSize / 2) / kernelSize);

                int y_out = std::clamp(y - radius, 0, height_ - 1);
                int y_in = std::clamp(y + radius + 1, 0, height_ - 1);

                const uint8_t* row_out = buffer1_.rowPtr(y_out);
                const uint8_t* row_in = buffer1_.rowPtr(y_in);

                sum -= row_out[x * channels + c];
                sum += row_in[x * channels + c];
            }
        }
    }

    // alpha à 255 si besoin
    if (channels == 4)
    {
        for (int y = 0; y < height_; ++y)
        {
            uint8_t* dst = buffer2_.rowPtr(y);
            for (int x = 0; x < width_; ++x)
                dst[x * channels + 3] = 255;
        }
    }

    std::swap(buffer1_, buffer2_);
}

ImageView MeanSliding::outputView() const
{
    return buffer1_.view();
}

const ImageOwner& MeanSliding::output() const
{
    return buffer1_;
}

} // namespace fluvel_ip
