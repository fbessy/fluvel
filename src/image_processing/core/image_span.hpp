// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

namespace fluvel_ip
{

enum class ImageFormat
{
    Gray8,
    Rgb24,
    Bgr24,
    Bgr32,
    Rgba32,
    Yuyv422,
    Nv12,
    Nv21,
    I420
};

class ImageSpan final
{
public:
    ImageSpan() = default;

    ImageSpan(const unsigned char* data, int widthPixels, int heightPixels,
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
    int size() const noexcept
    {
        return widthPixels_ * heightPixels_;
    }
    ImageFormat format() const noexcept
    {
        return format_;
    }
    int channels() const noexcept
    {
        return channelsPerPixel_;
    }
    int strideBytes() const noexcept
    {
        return strideBytes_;
    }

    const unsigned char* row(int y) const noexcept
    {
        assert(y >= 0 && y < heightPixels_);
        return data_ + static_cast<ptrdiff_t>(y * strideBytes_);
    }

    // ⚠️ Ne PAS utiliser pour YUYV
    unsigned char at(int x, int y, int c = 0) const noexcept
    {
        assert(format_ != ImageFormat::Yuyv422);
        assert(x >= 0 && x < widthPixels_);
        assert(y >= 0 && y < heightPixels_);
        assert(c >= 0 && c < channelsPerPixel_);

        return row(y)[x * channelsPerPixel_ + c];
    }

    // ---------------- RGB (affichage uniquement)
    inline Rgb_uc atPixelRgb(int x, int y) const noexcept
    {
        const unsigned char* p = row(y);

        switch (format_)
        {
            case ImageFormat::Rgb24:
            {
                const unsigned char* px = p + x * 3;
                return {px[0], px[1], px[2]};
            }
            case ImageFormat::Bgr24:
            {
                const unsigned char* px = p + x * 3;
                return {px[2], px[1], px[0]};
            }
            case ImageFormat::Bgr32:
            {
                const unsigned char* px = p + x * 4;
                return {px[2], px[1], px[0]};
            }
            case ImageFormat::Rgba32:
            {
                const unsigned char* px = p + x * 4;
                return {px[0], px[1], px[2]};
            }

            case ImageFormat::Gray8:
            case ImageFormat::Yuyv422:
            case ImageFormat::Nv12:
            case ImageFormat::Nv21:
            case ImageFormat::I420:
                std::unreachable();
        }

        std::unreachable();
    }

    // ---------------- API algo (source de vérité)
    inline Components_3i atPixel(int x, int y) const noexcept
    {
        const unsigned char* p = row(y);

        switch (format_)
        {
            case ImageFormat::Gray8:
            {
                int v = p[x];
                return {v, v, v};
            }

            case ImageFormat::Rgb24:
            {
                const unsigned char* px = p + x * 3;
                return {px[0], px[1], px[2]};
            }

            case ImageFormat::Bgr24:
            {
                const unsigned char* px = p + x * 3;
                return {px[2], px[1], px[0]};
            }

            case ImageFormat::Bgr32:
            {
                const unsigned char* px = p + x * 4;
                return {px[2], px[1], px[0]};
            }

            case ImageFormat::Rgba32:
            {
                const unsigned char* px = p + x * 4;
                return {px[0], px[1], px[2]};
            }

            case ImageFormat::Yuyv422:
            {
                // alignement sur paire YUYV
                const int x_pair = x & ~1;
                const unsigned char* px = p + x_pair * 2;

                const int Y = (x & 1) ? px[2] : px[0];
                const int U = px[1];
                const int V = px[3];

                return {Y, U - 128, V - 128};
            }

            case ImageFormat::Nv12:
            case ImageFormat::Nv21:
            {
                const int W = widthPixels_;
                const int H = heightPixels_;

                const unsigned char* Y_plane = data_;
                const unsigned char* UV_plane = data_ + W * H;

                int Y = Y_plane[y * W + x];

                int uv_x = x & ~1;
                int uv_y = y >> 1;

                const unsigned char* uv = UV_plane + uv_y * W + uv_x;

                const bool isNV21 = (format_ == ImageFormat::Nv21);

                int U = uv[isNV21 ? 1 : 0];
                int V = uv[isNV21 ? 0 : 1];

                return {Y, U - 128, V - 128};
            }

            case ImageFormat::I420:
            {
                const int W = widthPixels_;
                const int H = heightPixels_;

                const int wHalf = W >> 1;
                const int hHalf = H >> 1;

                const unsigned char* Y_plane = data_;
                const unsigned char* U_plane = Y_plane + W * H;
                const unsigned char* V_plane = U_plane + wHalf * hHalf;

                int Y = Y_plane[y * W + x];

                int uv_x = x >> 1;
                int uv_y = y >> 1;

                int idx = uv_y * wHalf + uv_x;

                int U = U_plane[idx];
                int V = V_plane[idx];

                return {Y, U - 128, V - 128};
            }
        }

        std::unreachable();
    }

    // ---------------- Luminance rapide
    unsigned char gray(int x, int y) const noexcept
    {
        const unsigned char* p = row(y);

        switch (format_)
        {
            case ImageFormat::Gray8:
                return p[x];

            case ImageFormat::Yuyv422:
                return p[x * 2];

            case ImageFormat::Nv12:
            case ImageFormat::Nv21:
            case ImageFormat::I420:
            {
                const int W = widthPixels_;
                return data_[y * W + x]; // Y plane
            }

            case ImageFormat::Rgb24:
            {
                const unsigned char* px = p + x * 3;
                return (77 * px[0] + 150 * px[1] + 29 * px[2]) >> 8;
            }

            case ImageFormat::Bgr24:
            {
                const unsigned char* px = p + x * 3;
                return (77 * px[2] + 150 * px[1] + 29 * px[0]) >> 8;
            }

            case ImageFormat::Bgr32:
            {
                const unsigned char* px = p + x * 4;
                return (77 * px[2] + 150 * px[1] + 29 * px[0]) >> 8;
            }

            case ImageFormat::Rgba32:
            {
                const unsigned char* px = p + x * 4;
                return (77 * px[0] + 150 * px[1] + 29 * px[2]) >> 8;
            }
        }

        std::unreachable();
    }

private:
    const unsigned char* data_;
    int widthPixels_;
    int heightPixels_;
    ImageFormat format_;
    int channelsPerPixel_;
    int strideBytes_;

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
            case ImageFormat::Yuyv422:
                return 2;
            case ImageFormat::Nv12:
                return 1; // Y plane
            case ImageFormat::Nv21:
                return 1; // Y plane
            case ImageFormat::I420:
                return 1;
        }

        std::unreachable();
    }
};

} // namespace fluvel_ip
