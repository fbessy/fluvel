// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file video_format_utils.hpp
 * @brief Utilities for video pixel format string conversion.
 *
 * Provides helper functions to convert
 * QVideoFrameFormat::PixelFormat values into human-readable
 * strings for logging and user interface display.
 */

#pragma once

#include <QString>
#include <QVideoFrameFormat>

namespace fluvel::video_utils
{

/**
 * @brief Converts a Qt video pixel format to a human-readable string.
 *
 * Returns the full name associated with the given
 * QVideoFrameFormat::PixelFormat value.
 *
 * @param format Pixel format to convert.
 * @return Human-readable pixel format name.
 */
QString pixelFormatToString(QVideoFrameFormat::PixelFormat format);

/**
 * @brief Converts a Qt video pixel format to a short display string.
 *
 * Returns a compact representation intended for UI display,
 * such as "NV12", "YUV420" or "MJPEG".
 *
 * @param format Pixel format to convert.
 * @return Short pixel format name.
 */
QString pixelFormatToShortString(QVideoFrameFormat::PixelFormat format);

} // namespace fluvel::video_utils