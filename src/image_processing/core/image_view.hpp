// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "image_format.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

namespace fluvel_ip
{

class ImageView final
{
public:
    ImageView() = default;

    ImageView(const unsigned char* data, int widthPixels, int heightPixels,
              ImageFormat format = ImageFormat::Gray8, int strideBytes = 0)
        : data_(data)
        , widthPixels_(widthPixels)
        , heightPixels_(heightPixels)
        , format_(format)
        , channelsPerPixel_(channels_from_format(format))
        , strideBytes_(compute_stride(widthPixels_, channelsPerPixel_, strideBytes))
    {
        assert(data != nullptr);
        assert(widthPixels > 0);
        assert(heightPixels > 0);
        assert(strideBytes >= 0);

        assert(strideBytes_ >= widthPixels_ * channelsPerPixel_);
    }

    const unsigned char* data() const noexcept
    {
        return data_;
    }

    int width() const noexcept
    {
        return widthPixels_;
    }

    int height() const noexcept
    {
        return heightPixels_;
    }

    int pixelCount() const noexcept
    {
        return widthPixels_ * heightPixels_;
    }

    int byteSize() const noexcept
    {
        return strideBytes_ * heightPixels_;
    }

    ImageFormat format() const noexcept
    {
        return format_;
    }

    int channels() const noexcept
    {
        return channelsPerPixel_;
    }

    int stride() const noexcept
    {
        return strideBytes_;
    }

    const unsigned char* row(int y) const noexcept
    {
        assert(y >= 0 && y < heightPixels_);
        return data_ + static_cast<ptrdiff_t>(y * strideBytes_);
    }

    unsigned char at(int x, int y, int c = 0) const noexcept
    {
        assert(x >= 0 && x < widthPixels_);
        assert(y >= 0 && y < heightPixels_);
        assert(c >= 0 && c < channelsPerPixel_);

        return row(y)[x * channelsPerPixel_ + c];
    }

    inline Rgb_uc atPixelRgb(int x, int y) const noexcept
    {
        const unsigned char* p = row(y) + static_cast<ptrdiff_t>(x * channelsPerPixel_);

        switch (format_)
        {
            case ImageFormat::Gray8:
            {
                const unsigned char v = p[0];
                return {v, v, v};
            }
            case ImageFormat::Rgb24:
                return {p[0], p[1], p[2]};

            case ImageFormat::Bgr24:
                return {p[2], p[1], p[0]};

            case ImageFormat::Bgr32:
                return {p[2], p[1], p[0]};

            case ImageFormat::Rgba32:
                return {p[0], p[1], p[2]};
        }

        std::unreachable();
    }

    unsigned char gray(int x, int y) const noexcept
    {
        assert(format_ == ImageFormat::Gray8);
        assert(channelsPerPixel_ == 1);

        return at(x, y, 0);
    }

private:
    const unsigned char* data_;
    int widthPixels_{0};
    int heightPixels_{0};
    ImageFormat format_{ImageFormat::Gray8};
    int channelsPerPixel_{0};
    int strideBytes_{0}; // bytes per row

    static int compute_stride(int width, int channels, int strideBytes)
    {
        return (strideBytes == 0) ? width * channels : strideBytes;
    }

    static constexpr int channels_from_format(ImageFormat fmt) noexcept
    {
        switch (fmt)
        {
            case ImageFormat::Gray8:
                return 1;
            case ImageFormat::Rgb24:
                return 3;
            case ImageFormat::Bgr24:
                return 3;
            case ImageFormat::Bgr32:
                return 4;
            case ImageFormat::Rgba32:
                return 4;
        }

        std::unreachable();
    }
};

} // namespace fluvel_ip
