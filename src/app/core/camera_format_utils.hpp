// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file camera_format_utils.hpp
 * @brief Utility functions related to camera device and formats.
 *
 * This module provides helper functions to compare and manipulate
 * Qt camera-related types (e.g. QCameraFormat).
 */

#pragma once

#include <QCameraFormat>
#include <QString>
#include <QVideoFrameFormat>

namespace fluvel::camera_utils
{

/**
 * @brief Compare two QCameraFormat objects for practical equivalence.
 *
 * This function checks whether two camera formats can be considered identical
 * for usage in the application. It compares:
 * - pixel format
 * - resolution
 * - frame rate (with tolerance)
 *
 * A small epsilon is used for frame rate comparison to account for floating-point
 * inaccuracies and minor differences between reported formats.
 *
 * @param a First camera format.
 * @param b Second camera format.
 * @return true if formats are considered equivalent, false otherwise.
 */
bool isSameCameraFormat(const QCameraFormat& a, const QCameraFormat& b);

/**
 * @brief Returns whether the pixel format is a YUV 4:2:0 format.
 *
 * Supported formats include YUV420P, NV12 and NV21.
 *
 * @param format Pixel format to test.
 * @return True if the format is YUV 4:2:0.
 */
bool isYuv420(QVideoFrameFormat::PixelFormat format);

/**
 * @brief Returns whether the pixel format is a YUV format.
 *
 * @param format Pixel format to test.
 * @return True if the format is supported as YUV.
 */
bool isYuv(QVideoFrameFormat::PixelFormat format);

/**
 * @brief Returns whether the resolution is 640×480.
 *
 * @param size Resolution to test.
 * @return True if the resolution is 640×480.
 */
bool is640x480(const QSize& size);

/**
 * @brief Returns whether the frame rate is approximately 30 fps.
 *
 * @param fps Frame rate to test.
 * @return True if the frame rate is close to 30 fps.
 */
bool is30fps(float fps);

/**
 * @brief Finds the preferred camera format in a list of formats.
 *
 * The selection prioritizes YUV 4:2:0 formats at 640×480 and
 * 30 fps when available.
 *
 * @param formats Available camera formats.
 * @return Index of the preferred format, or -1 if no suitable format is found.
 */
int findBestFormatIndex(const QList<QCameraFormat>& formats);

/**
 * @brief Converts a camera format to a display string.
 *
 * The resulting string is intended for UI display and includes
 * the pixel format, resolution and frame rate when available.
 *
 * @param format Camera format to convert.
 * @return Display string for the camera format.
 */
QString formatToString(const QCameraFormat& format);

} // namespace fluvel::camera_utils
