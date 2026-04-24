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

/**
 * @brief Owning image container storing pixel data in contiguous memory.
 *
 * ImageOwner manages memory allocation and provides mutable access
 * to image pixels. It is designed to work alongside ImageView,
 * which provides a non-owning view of the data.
 *
 * The image is stored in row-major order with tightly packed rows:
 * stride = width * channels.
 *
 * Typical usage:
 * @code
 * ImageOwner img(w, h, ImageFormat::Rgb24);
 * auto view = img.view();
 * @endcode
 */
class ImageOwner
{
public:
    /// Default constructor (empty image).
    ImageOwner() = default;

    /**
     * @brief Construct an image with given dimensions and format.
     *
     * @param widthPixels Image width.
     * @param heightPixels Image height.
     * @param format Pixel format.
     *
     * @pre widthPixels > 0
     * @pre heightPixels > 0
     */
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

    /**
     * @brief Create an ImageOwner with the same layout as a view.
     */
    static ImageOwner like(const ImageView& view)
    {
        return ImageOwner(view.width(), view.height(), view.format());
    }

    /**
     * @brief Check if the layout matches another image view.
     *
     * Compares width, height, and format.
     */
    bool hasSameLayout(const ImageView& view) const noexcept
    {
        return width() == view.width() && height() == view.height() && format() == view.format();
    }

    /**
     * @brief Ensure this image matches the layout of a view.
     *
     * Reallocates if needed.
     */
    void ensureLike(const ImageView& view)
    {
        if (!hasSameLayout(view))
            *this = ImageOwner::like(view);
    }

    /**
     * @brief Copy pixel data from a view.
     *
     * Performs a row-by-row memcpy.
     */
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

    /**
     * @brief Get pointer to a row (mutable).
     */
    uint8_t* rowPtr(int y) noexcept
    {
        assert(y >= 0 && y < height_);
        return data_.data() + y * stride_;
    }

    /**
     * @brief Get pointer to a row (const).
     */
    const uint8_t* rowPtr(int y) const noexcept
    {
        assert(y >= 0 && y < height_);
        return data_.data() + y * stride_;
    }

    /**
     * @brief Get number of bytes per row (excluding padding).
     */
    int rowBytes() const noexcept
    {
        return width_ * channels_;
    }

    /**
     * @brief Get pointer to a pixel (mutable).
     */
    uint8_t* pixelPtr(int x, int y) noexcept
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);

        return data_.data() + y * stride_ + x * channels_;
    }

    /**
     * @brief Get pointer to a pixel (const).
     */
    const uint8_t* pixelPtr(int x, int y) const noexcept
    {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);

        return data_.data() + y * stride_ + x * channels_;
    }

    /**
     * @brief Access pixel channel with bounds checking (assert).
     */
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

    /**
     * @brief Safe access with exception on out-of-bounds.
     *
     * @throws std::out_of_range if indices are invalid.
     */
    uint8_t& atSafe(int x, int y, int c = 0)
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_ || c < 0 || c >= channels_)
            throw std::out_of_range("Image::at_safe - index out of bounds");

        const size_t idx = static_cast<size_t>(y) * static_cast<size_t>(stride_) +
                           static_cast<size_t>(x) * static_cast<size_t>(channels_) +
                           static_cast<size_t>(c);

        return data_[idx];
    }

    /**
     * @brief Get raw data pointer (mutable).
     */
    uint8_t* data() noexcept
    {
        return data_.data();
    }

    /**
     * @brief Get raw data pointer (const).
     */
    const uint8_t* data() const noexcept
    {
        return data_.data();
    }

    /**
     * @brief Get image width.
     */
    int width() const noexcept
    {
        return width_;
    }

    /**
     * @brief Get image height.
     */
    int height() const noexcept
    {
        return height_;
    }

    /**
     * @brief Get row stride in bytes.
     */
    int stride() const noexcept
    {
        return stride_;
    }

    /**
     * @brief Get pixel format.
     */
    ImageFormat format() const noexcept
    {
        return format_;
    }

    /**
     * @brief Get a non-owning view of the image.
     */
    ImageView view() const noexcept
    {
        return ImageView(data(), width_, height_, format_, stride_);
    }

private:
    /// Image width in pixels.
    int width_{0};

    /// Image height in pixels.
    int height_{0};

    /// Pixel format.
    ImageFormat format_{ImageFormat::Gray8};

    /// Number of channels per pixel.
    int channels_{0};

    /// Row stride in bytes.
    int stride_{0};

    /// Pixel data (row-major).
    std::vector<uint8_t> data_;

    /**
     * @brief Get number of channels from image format.
     */
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
