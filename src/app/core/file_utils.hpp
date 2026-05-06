// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file file_utils.hpp
 * @brief Utility functions for file handling and image filtering.
 *
 * This module provides helper functions for:
 * - building file dialog filters
 * - checking supported image formats
 * - manipulating file names
 */

#pragma once

#include <QString>

namespace fluvel_app::file_utils
{

/**
 * @brief Build a file dialog filter for supported image formats.
 *
 * Typically used with QFileDialog to restrict selectable image types.
 *
 * @return A filter string (e.g. "Images (*.png *.jpg ...)").
 */
QString buildImageFilter();

/**
 * @brief Builds a string containing all supported image file extensions.
 *
 * The returned extensions are queried dynamically from Qt image plugins
 * using QImageReader::supportedImageFormats().
 *
 * Example:
 * "*.bmp *.jpg *.png *.tif"
 *
 * @return Space-separated list of supported image file extensions.
 */
QString supportedImageExtensions();

/**
 * @brief Check whether a file is a supported image.
 *
 * @param path File path to test.
 * @return true if the file extension or format is supported.
 */
bool isSupportedImage(const QString& path);

/**
 * @brief Extract the base name of a file without path.
 *
 * @param fullFilename Full file path.
 * @return File name without directory components.
 */
QString strippedName(const QString& fullFilename);

/**
 * @brief Generate a unique file name from a base path.
 *
 * If the file already exists, a suffix is typically added to avoid overwriting.
 *
 * @param filePath Input file path.
 * @return A modified file path that does not collide with existing files.
 */
QString makeUniqueFileName(const QString& filePath);

} // namespace fluvel_app::file_utils
