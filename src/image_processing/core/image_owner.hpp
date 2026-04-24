// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_format.hpp"
#include "image_view.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace fluvel_ip
{

class ImageOwner
{
public:
    ImageOwner() = default;

    ImageOwner(int widthPixels, int heightPixels, ImageFormat format)
        : width_(widthPixels)
        , height_(heightPixels)
        , format_(format)
        , channels_(channelsFromFormat(format))
        , stride_(widthPixels * channels_)
        , data_(static_cast<size_t>(stride_) * static_cast<size_t>(heightPixels))
    {
        assert(widthPixels > 0);
        assert(heightPixels > 0);
        assert(channels_ > 0);
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
            uint8_t* dst = rowPtr(y);

            std::memcpy(dst, src, rowBytes);
        }
    }

    uint8_t* rowPtr(int y) noexcept
    {
        assert(y >= 0 && y < height_);
        return data_.data() + y * stride_;
    }

    const uint8_t* rowPtr(int y) const noexcept
    {
        assert(y >= 0 && y < height_);
        return data_.data() + y * stride_;
    }

    int rowBytes() const noexcept
    {
        return width_ * channels_;
    }

    uint8_t* pixelPtr(int x, int y) noexcept
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);

        return data_.data() + y * stride_ + x * channels_;
    }

    const uint8_t* pixelPtr(int x, int y) const noexcept
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);

        return data_.data() + y * stride_ + x * channels_;
    }

    uint8_t& at(int x, int y, int c = 0) noexcept
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);
        assert(c >= 0 && c < channels_);

        const size_t idx = static_cast<size_t>(y) * static_cast<size_t>(stride_) +
                           static_cast<size_t>(x) * static_cast<size_t>(channels_) +
                           static_cast<size_t>(c);

        return data_[idx];
    }

    uint8_t& atSafe(int x, int y, int c = 0)
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_ || c < 0 || c >= channels_)
            throw std::out_of_range("Image::at_safe - index out of bounds");

        const size_t idx = static_cast<size_t>(y) * static_cast<size_t>(stride_) +
                           static_cast<size_t>(x) * static_cast<size_t>(channels_) +
                           static_cast<size_t>(c);

        return data_[idx];
    }

    uint8_t* data() noexcept
    {
        return data_.data();
    }
    const uint8_t* data() const noexcept
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
    std::vector<uint8_t> data_;

    static constexpr int channelsFromFormat(ImageFormat fmt) noexcept
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
