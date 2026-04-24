// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

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

} // namespace fluvel_ip
