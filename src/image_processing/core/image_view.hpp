// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "image_format.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace fluvel_ip
{

/**
 * @brief Non-owning view over image data.
 *
 * ImageView provides read-only access to image pixels without owning the memory.
 * It is typically created from external buffers or from ImageOwner via view().
 *
 * The data is assumed to be stored in row-major order, with a configurable stride.
 *
 * @warning The user is responsible for ensuring the lifetime of the underlying data.
 */
class ImageView final
{
public:
    /// Default constructor (empty view).
    ImageView() = default;

    /**
     * @brief Construct an image view from raw data.
     *
     * @param data Pointer to image buffer.
     * @param widthPixels Image width.
     * @param heightPixels Image height.
     * @param format Pixel format.
     * @param strideBytes Number of bytes per row (0 = tightly packed).
     *
     * @pre data != nullptr
     * @pre widthPixels > 0
     * @pre heightPixels > 0
     * @pre strideBytes >= 0
     */
    ImageView(const unsigned char* data, int widthPixels, int heightPixels,
              ImageFormat format = ImageFormat::Gray8, int strideBytes = 0)
        : data_(reinterpret_cast<const uint8_t*>(data))
        , widthPixels_(widthPixels)
        , heightPixels_(heightPixels)
        , format_(format)
        , channelsPerPixel_(channelsFromFormat(format))
        , strideBytes_(computeStride(widthPixels_, channelsPerPixel_, strideBytes))
    {
        assert(data != nullptr);
        assert(widthPixels > 0);
        assert(heightPixels > 0);
        assert(strideBytes >= 0);

        assert(strideBytes_ >= widthPixels_ * channelsPerPixel_);
    }

    /**
     * @brief Get raw data pointer.
     */
    const uint8_t* data() const noexcept
    {
        return data_;
    }

    /**
     * @brief Get image width.
     */
    int width() const noexcept
    {
        return widthPixels_;
    }

    /**
     * @brief Get image height.
     */
    int height() const noexcept
    {
        return heightPixels_;
    }

    /**
     * @brief Get total number of pixels.
     */
    int pixelCount() const noexcept
    {
        return widthPixels_ * heightPixels_;
    }

    /**
     * @brief Get total size in bytes.
     */
    int byteSize() const noexcept
    {
        return strideBytes_ * heightPixels_;
    }

    /**
     * @brief Get pixel format.
     */
    ImageFormat format() const noexcept
    {
        return format_;
    }

    /**
     * @brief Get number of channels per pixel.
     */
    int channels() const noexcept
    {
        return channelsPerPixel_;
    }

    /**
     * @brief Get row stride in bytes.
     */
    int stride() const noexcept
    {
        return strideBytes_;
    }

    /**
     * @brief Get pointer to a row.
     */
    const uint8_t* row(int y) const noexcept
    {
        assert(y >= 0 && y < heightPixels_);
        return data_ + static_cast<ptrdiff_t>(y * strideBytes_);
    }

    /**
     * @brief Get pointer to a row with clamped index.
     */
    const uint8_t* rowClamped(int y) const noexcept
    {
        return row(std::clamp(y, 0, heightPixels_ - 1));
    }

    /**
     * @brief Access pixel channel with bounds checking (assert).
     */
    uint8_t at(int x, int y, int c = 0) const noexcept
    {
        assert(x >= 0 && x < widthPixels_);
        assert(y >= 0 && y < heightPixels_);
        assert(c >= 0 && c < channelsPerPixel_);

        return row(y)[x * channelsPerPixel_ + c];
    }

    /**
     * @brief Access pixel channel with clamped coordinates.
     */
    uint8_t atClamped(int x, int y, int c = 0) const noexcept
    {
        x = std::clamp(x, 0, widthPixels_ - 1);
        y = std::clamp(y, 0, heightPixels_ - 1);

        return row(y)[x * channelsPerPixel_ + c];
    }

    /**
     * @brief Read pixel as RGB (8-bit per channel).
     *
     * Converts from internal format (Gray, RGB, BGR, etc.)
     * to a unified RGB representation.
     */
    inline Rgb_uc atPixelRgb(int x, int y) const noexcept
    {
        const uint8_t* p = row(y) + static_cast<ptrdiff_t>(x * channelsPerPixel_);

        switch (format_)
        {
            case ImageFormat::Gray8:
            {
                const uint8_t v = p[0];
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

    /**
     * @brief Read pixel components as float RGB values.
     *
     * @param row Pointer to the row.
     * @param idx Pixel index within the row (in bytes).
     * @param r Output red channel.
     * @param g Output green channel.
     * @param b Output blue channel.
     */
    inline void readPixelRgb(const uint8_t* row, int idx, float& r, float& g,
                             float& b) const noexcept
    {
        switch (format_)
        {
            case ImageFormat::Gray8:
            {
                float v = row[idx];
                r = g = b = v;
                break;
            }
            case ImageFormat::Rgb24:
            case ImageFormat::Rgba32:
            {
                r = row[idx + 0];
                g = row[idx + 1];
                b = row[idx + 2];
                break;
            }
            case ImageFormat::Bgr24:
            case ImageFormat::Bgr32:
            {
                b = row[idx + 0];
                g = row[idx + 1];
                r = row[idx + 2];
                break;
            }
        }
    }

    /**
     * @brief Get grayscale value (Gray8 only).
     *
     * @pre format() == ImageFormat::Gray8
     */
    uint8_t gray(int x, int y) const noexcept
    {
        assert(format_ == ImageFormat::Gray8);
        assert(channelsPerPixel_ == 1);

        return at(x, y, 0);
    }

private:
    /// Pointer to image data (non-owning).
    const uint8_t* data_;

    /// Image width.
    int widthPixels_{0};

    /// Image height.
    int heightPixels_{0};

    /// Pixel format.
    ImageFormat format_{ImageFormat::Gray8};

    /// Number of channels per pixel.
    int channelsPerPixel_{0};

    /// Row stride in bytes.
    int strideBytes_{0};

    /**
     * @brief Compute stride (default = tightly packed).
     */
    static int computeStride(int width, int channels, int strideOverride)
    {
        return (strideOverride == 0) ? width * channels : strideOverride;
    }

    /**
     * @brief Get number of channels from format.
     */
    static constexpr int channelsFromFormat(ImageFormat format) noexcept
    {
        switch (format)
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
