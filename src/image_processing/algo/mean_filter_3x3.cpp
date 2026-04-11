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
    const int w = input.width();
    const int h = input.height();
    const auto fmt = input.format();

    if (buffer1_.width() != w || buffer1_.height() != h || buffer1_.format() != fmt)
    {
        buffer1_ = ImageOwner(w, h, fmt);
        buffer2_ = ImageOwner(w, h, fmt);
    }

    width_ = w;
    height_ = h;
}

void Mean3x3::apply(const ImageView& input)
{
    reset(input);

    const int stride1 = buffer1_.stride();
    const int stride2 = buffer2_.stride();
    const int channels = input.channels();

    // =========================
    // PASS 1 — Horizontal
    // =========================
    for (int y = 0; y < height_; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = buffer1_.data() + y * stride1;

        horizontalPass(src, dst, width_, channels);
    }

    // =========================
    // PASS 2 — Vertical
    // =========================
    for (int y = 0; y < height_; ++y)
    {
        const uint8_t* row_m1 = buffer1_.data() + std::max(0, y - 1) * stride1;
        const uint8_t* row_0 = buffer1_.data() + y * stride1;
        const uint8_t* row_p1 = buffer1_.data() + std::min(height_ - 1, y + 1) * stride1;

        uint8_t* dst = buffer2_.data() + y * stride2;

        verticalPass(row_m1, row_0, row_p1, dst, width_, channels);
    }

    std::swap(buffer1_, buffer2_);
}

void Mean3x3::horizontalPass(const uint8_t* src, uint8_t* dst, int width, int channels)
{
    const int rowSize = width * channels;

    // --- left border
    for (int c = 0; c < channels; ++c)
    {
        int sum = src[c] + src[c] + src[channels + c];
        dst[c] = (sum + 1) / 3;
    }

    // --- center
    for (int i = channels; i < rowSize - channels; ++i)
    {
        int sum = src[i - channels] + src[i] + src[i + channels];
        dst[i] = (sum + 1) / 3;
    }

    // --- right border
    int base = rowSize - channels;
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
    const int rowSize = width * channels;

    for (int i = 0; i < rowSize; ++i)
    {
        int sum = row_m1[i] + row_0[i] + row_p1[i];
        dst[i] = (sum + 1) / 3;
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
