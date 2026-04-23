// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "mean_filter_3x3.hpp"

namespace fluvel_ip::filter
{

void mean3x3(const ImageView& input, ImageOwner& output)
{
    Mean3x3 impl;

    impl.reset(input);
    impl.apply(input);

    output.copyFrom(impl.outputView());
}

ImageOwner mean3x3(const ImageView& input)
{
    Mean3x3 impl;

    impl.reset(input);
    impl.apply(input);

    return impl.output(); // copy (safe)
}

void Mean3x3::reset(const ImageView& input)
{
    if (!buffer1_.hasSameLayout(input))
    {
        buffer1_ = ImageOwner::like(input);
        buffer2_ = ImageOwner::like(input);
    }

    width_ = input.width();
    height_ = input.height();
}

void Mean3x3::apply(const ImageView& input)
{
    reset(input);

    const int channels = input.channels();

    // =========================
    // PASS 1 — Horizontal
    // =========================
    for (int y = 0; y < height_; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = buffer1_.rowPtr(y);

        horizontalPass(src, dst, width_, channels);
    }

    // =========================
    // PASS 2 — Vertical
    // =========================
    for (int y = 0; y < height_; ++y)
    {
        const uint8_t* row_m1 = buffer1_.rowPtr(std::max(0, y - 1));
        const uint8_t* row_0 = buffer1_.rowPtr(y);
        const uint8_t* row_p1 = buffer1_.rowPtr(std::min(height_ - 1, y + 1));

        uint8_t* dst = buffer2_.rowPtr(y);

        verticalPass(row_m1, row_0, row_p1, dst, width_, channels);
    }

    std::swap(buffer1_, buffer2_);
}

void Mean3x3::horizontalPass(const uint8_t* src, uint8_t* dst, int width, int channels)
{
    if (width == 1)
    {
        for (int c = 0; c < channels; ++c)
            dst[c] = src[c];
        return;
    }

    const int rowBytes = width * channels;

    // --- left border
    for (int c = 0; c < channels; ++c)
    {
        int sum = src[c] + src[c] + src[channels + c];
        dst[c] = (sum + 1) / 3;
    }

    // --- center
    for (int idx = channels; idx < rowBytes - channels; ++idx)
    {
        int sum = src[idx - channels] + src[idx] + src[idx + channels];
        dst[idx] = (sum + 1) / 3;
    }

    // --- right border
    const int base = rowBytes - channels;
    for (int c = 0; c < channels; ++c)
    {
        int idx = base + c;
        int sum = src[idx - channels] + src[idx] + src[idx];
        dst[idx] = (sum + 1) / 3;
    }
}

void Mean3x3::verticalPass(const uint8_t* row_m1, const uint8_t* row_0, const uint8_t* row_p1,
                           uint8_t* dst, int width, int channels)
{
    const int rowBytes = width * channels;

    for (int idx = 0; idx < rowBytes; ++idx)
    {
        int sum = row_m1[idx] + row_0[idx] + row_p1[idx];
        dst[idx] = (sum + 1) / 3;
    }
}

ImageView Mean3x3::outputView() const
{
    return buffer1_.view();
}

const ImageOwner& Mean3x3::output() const
{
    return buffer1_;
}

} // namespace fluvel_ip
