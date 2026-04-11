#include "spatial_filter.hpp"

namespace fluvel_ip::filter
{

void spatialFilter(const ImageView& input, ImageOwner& output)
{
    SpatialFilter impl;

    impl.reset(input);
    impl.apply(input);

    output.copyFrom(impl.outputView());
}

ImageOwner spatialFilter(const ImageView& input)
{
    SpatialFilter impl;

    impl.reset(input);
    impl.apply(input);

    return impl.output(); // copy (safe)
}

void SpatialFilter::reset(const ImageView& input)
{
    const int w = input.width();
    const int h = input.height();

    if (buffer1_.width() != w || buffer1_.height() != h || buffer1_.format() != ImageFormat::Bgr32)
    {
        buffer1_ = ImageOwner(w, h, ImageFormat::Bgr32);
        buffer2_ = ImageOwner(w, h, ImageFormat::Bgr32);
    }

    width_ = w;
    height_ = h;
}

void SpatialFilter::apply(const ImageView& input)
{
    reset(input);

    const int stride1 = buffer1_.stride();
    const int stride2 = buffer2_.stride();

    // =========================
    // PASS 1 — Horizontal
    // =========================
    for (int y = 0; y < height_; ++y)
    {
        const unsigned char* src = input.row(y);
        unsigned char* dst = buffer1_.data() + y * stride1;

        // --- left border (x = 0)
        {
            for (int c = 0; c < 3; ++c)
            {
                int sum = src[c] + src[c] + src[4 + c];
                dst[c] = static_cast<unsigned char>((sum + 1) / 3);
            }
            dst[3] = 255;
        }

        // --- center (no clamp)
        for (int x = 1; x < width_ - 1; ++x)
        {
            int base = 4 * x;

            for (int c = 0; c < 3; ++c)
            {
                int sum = src[base - 4 + c] + src[base + c] + src[base + 4 + c];

                dst[base + c] = static_cast<unsigned char>((sum + 1) / 3);
            }

            dst[base + 3] = 255;
        }

        // --- right border (x = width - 1)
        {
            int base = 4 * (width_ - 1);

            for (int c = 0; c < 3; ++c)
            {
                int sum = src[base - 4 + c] + src[base + c] + src[base + c];

                dst[base + c] = static_cast<unsigned char>((sum + 1) / 3);
            }

            dst[base + 3] = 255;
        }
    }

    // =========================
    // PASS 2 — Vertical
    // =========================

    // --- top row (y = 0)
    {
        unsigned char* dst = buffer2_.data();

        const unsigned char* row0 = buffer1_.data();
        const unsigned char* row1 = buffer1_.data() + stride1;

        for (int x = 0; x < width_; ++x)
        {
            int base = 4 * x;

            for (int c = 0; c < 3; ++c)
            {
                int sum = row0[base + c] + row0[base + c] + row1[base + c];

                dst[base + c] = static_cast<unsigned char>((sum + 1) / 3);
            }

            dst[base + 3] = 255;
        }
    }

    // --- center rows (no clamp)
    for (int y = 1; y < height_ - 1; ++y)
    {
        unsigned char* dst = buffer2_.data() + y * stride2;

        const unsigned char* row_m1 = buffer1_.data() + (y - 1) * stride1;
        const unsigned char* row_0 = buffer1_.data() + y * stride1;
        const unsigned char* row_p1 = buffer1_.data() + (y + 1) * stride1;

        for (int x = 0; x < width_; ++x)
        {
            int base = 4 * x;

            for (int c = 0; c < 3; ++c)
            {
                int sum = row_m1[base + c] + row_0[base + c] + row_p1[base + c];

                dst[base + c] = static_cast<unsigned char>((sum + 1) / 3);
            }

            dst[base + 3] = 255;
        }
    }

    // --- bottom row (y = height - 1)
    {
        int y = height_ - 1;

        unsigned char* dst = buffer2_.data() + y * stride2;

        const unsigned char* row_m1 = buffer1_.data() + (y - 1) * stride1;
        const unsigned char* row_0 = buffer1_.data() + y * stride1;

        for (int x = 0; x < width_; ++x)
        {
            int base = 4 * x;

            for (int c = 0; c < 3; ++c)
            {
                int sum = row_m1[base + c] + row_0[base + c] + row_0[base + c];

                dst[base + c] = static_cast<unsigned char>((sum + 1) / 3);
            }

            dst[base + 3] = 255;
        }
    }

    // =========================
    // FINAL SWAP
    // =========================
    std::swap(buffer1_, buffer2_);
}

ImageView SpatialFilter::outputView() const
{
    return buffer1_.view();
}

const ImageOwner& SpatialFilter::output() const
{
    return buffer1_;
}

} // namespace fluvel_ip
