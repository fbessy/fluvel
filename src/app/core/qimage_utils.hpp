// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file qimage_utils.hpp
 * @brief Utilities for QImage manipulation and hashing.
 *
 * This module provides helper functions to:
 * - apply simple visual transformations (e.g. darkening)
 * - compute image hashes for caching, comparison, or debugging
 *
 * These utilities are intended for UI and lightweight processing tasks.
 */

#pragma once

#include <QByteArray>
#include <QImage>

#include <string>

namespace fluvel_app::qimage_utils
{

/**
 * @brief Darken an image.
 *
 * Applies a global darkening effect to the input image.
 * Typically used for UI effects (e.g. background dimming, disabled state).
 *
 * @param image Input image.
 * @return Darkened image.
 */
QImage darkenImage(const QImage& image);

/**
 * @brief Compute a binary hash of an image.
 *
 * Generates a hash value from the image content.
 * Useful for change detection, caching, or debugging.
 *
 * @param image Input image.
 * @return Hash as a byte array.
 */
QByteArray imageHash(const QImage& image);

/**
 * @brief Compute a hexadecimal hash string of an image.
 *
 * Convenience wrapper around imageHash(), returning a human-readable
 * hexadecimal representation.
 *
 * @param image Input image.
 * @return Hash as a hexadecimal string.
 */
std::string hexHash(const QImage& image);

} // namespace fluvel_app::qimage_utils
