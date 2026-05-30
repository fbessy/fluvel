// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <utility>

namespace fluvel_ip
{

/**
 * @brief Enumeration of supported image pixel formats.
 *
 * Defines how pixel data is stored in memory, including:
 * - number of channels
 * - channel order
 * - bytes per pixel
 */
enum class ImageFormat
{
    /**
     * @brief Unsupported or unspecified image format.
     */
    Unknown,

    /**
     * @brief 8-bit grayscale image.
     *
     * - 1 channel
     * - Layout: [Gray]
     * - 1 byte per pixel
     */
    Gray8,

    /**
     * @brief 24-bit RGB image.
     *
     * - 3 channels
     * - Layout: [R, G, B]
     * - 3 bytes per pixel
     */
    Rgb24,

    /**
     * @brief 24-bit BGR image.
     *
     * - 3 channels
     * - Layout: [B, G, R]
     * - 3 bytes per pixel
     */
    Bgr24,

    /**
     * @brief 32-bit BGR image with padding or alpha.
     *
     * - 4 channels
     * - Layout: [B, G, R, A]
     * - 4 bytes per pixel
     *
     * @note The alpha channel may be unused depending on context.
     */
    Bgr32,

    /**
     * @brief 32-bit RGBA image.
     *
     * - 4 channels
     * - Layout: [R, G, B, A]
     * - 4 bytes per pixel
     */
    Rgba32
};

/**
 * @brief Returns the number of channels associated with an image format.
 *
 * Maps a valid ImageFormat value to its corresponding channel count.
 *
 * Supported mappings:
 * - Gray8 → 1
 * - Rgb24, Bgr24 → 3
 * - Bgr32, Rgba32 → 4
 *
 * @param format Image format.
 *
 * @return Number of channels associated with the format.
 *
 * @pre format must not be ImageFormat::Unknown.
 */
constexpr int channelCount(ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::Gray8:
            return 1;

        case ImageFormat::Rgb24:
        case ImageFormat::Bgr24:
            return 3;

        case ImageFormat::Bgr32:
        case ImageFormat::Rgba32:
            return 4;

        case ImageFormat::Unknown:
            break;
    }

    std::unreachable();
}

/**
 * @brief Returns the default image format associated with a channel count.
 *
 * Maps a channel count to Fluvel's default image format.
 *
 * Supported mappings:
 * - 1 → Gray8
 * - 3 → Rgb24
 * - 4 → Rgba32
 *
 * @param channels Number of image channels.
 *
 * @return Corresponding image format,
 *         or ImageFormat::Unknown if unsupported.
 */
constexpr ImageFormat formatFromChannelCount(int channels)
{
    switch (channels)
    {
        case 1:
            return ImageFormat::Gray8;

        case 3:
            return ImageFormat::Bgr24;

        case 4:
            return ImageFormat::Bgr32;

        default:
            return ImageFormat::Unknown;
    }
}

} // namespace fluvel_ip
