// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file image_debug.hpp
 * @brief Debug utilities for QImage inspection and logging.
 *
 * This module provides helper functions to:
 * - print QImage information to output streams
 * - generate human-readable descriptions of images
 *
 * Intended for debugging, logging, and diagnostics.
 */

#pragma once

#include <QImage>
#include <QString>

#include <iostream>

namespace fluvel_app::image_debug
{

/**
 * @brief Stream operator for QImage.
 *
 * Outputs a textual representation of the image, typically including
 * properties such as size, format, and possibly other metadata.
 *
 * @param os Output stream.
 * @param img Image to describe.
 * @return Reference to the output stream.
 */
std::ostream& operator<<(std::ostream& os, const QImage& img);

/**
 * @brief Generate a human-readable description of a QImage.
 *
 * Provides a string summary of the image, useful for logs or UI display.
 *
 * @param img Image to describe.
 * @return Description string.
 */
QString describeImage(const QImage& img);

} // namespace fluvel_app::image_debug
