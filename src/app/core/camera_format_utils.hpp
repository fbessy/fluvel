// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file camera_utils.hpp
 * @brief Utility functions related to camera configuration and formats.
 *
 * This module provides helper functions to compare and manipulate
 * Qt camera-related types (e.g. QCameraFormat).
 */

#pragma once

#include <QCameraFormat>
#include <cmath>

namespace fluvel_app::camera_utils
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
inline bool isSameCameraFormat(const QCameraFormat& a, const QCameraFormat& b)
{
    constexpr double kFpsEpsilon = 0.01;

    return a.pixelFormat() == b.pixelFormat() && a.resolution() == b.resolution() &&
           std::abs(a.maxFrameRate() - b.maxFrameRate()) < kFpsEpsilon;
}

} // namespace fluvel_app::camera_utils
