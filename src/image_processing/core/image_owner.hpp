// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_format.hpp"
#include "image_view.hpp"

#include <cstring>
#include <vector>

namespace fluvel_ip
{

class ImageOwner
{
public:
    ImageOwner() = default;

    ImageOwner(int w, int h, ImageFormat fmt)
        : width_(w)
        , height_(h)
        , format_(fmt)
        , channels_(channels_from_format(fmt))
        , stride_(w * channels_)
        , data_(static_cast<size_t>(stride_) * static_cast<size_t>(h))
    {
    }

    static ImageOwner like(const ImageView& view)
    {
        return ImageOwner(view.width(), view.height(), view.format());
    }

    bool hasSameLayout(const ImageView& view) const noexcept
    {
        return width() == view.width() && height() == view.height() && format() == view.format();
    }

    void ensureLike(const ImageView& view)
    {
        if (!hasSameLayout(view))
            *this = ImageOwner::like(view);
    }

    void copyFrom(const ImageView& img)
    {
        ensureLike(img);

        const int h = img.height();
        const std::size_t rowBytes = static_cast<std::size_t>(img.width() * img.channels());

        for (int y = 0; y < h; ++y)
        {
            const uint8_t* src = img.row(y);
            uint8_t* dst = data() + y * stride();

            std::memcpy(dst, src, rowBytes);
        }
    }

    unsigned char* data() noexcept
    {
        return data_.data();
    }
    const unsigned char* data() const noexcept
    {
        return data_.data();
    }

    int width() const noexcept
    {
        return width_;
    }

    int height() const noexcept
    {
        return height_;
    }

    int stride() const noexcept
    {
        return stride_;
    }

    ImageFormat format() const noexcept
    {
        return format_;
    }

    ImageView view() const noexcept
    {
        return ImageView(data(), width_, height_, format_, stride_);
    }

private:
    int width_{0};
    int height_{0};
    ImageFormat format_{ImageFormat::Gray8};
    int channels_{0};
    int stride_{0};
    std::vector<unsigned char> data_;

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
