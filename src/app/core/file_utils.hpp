// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file file_utils.hpp
 * @brief Utility functions for file and image-format handling.
 *
 * This module provides helper functions for:
 * - building file dialog filters
 * - querying supported image formats
 * - manipulating file names and paths
 */

#pragma once

#include <QString>

namespace fluvel::file_utils
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
 * @brief Normalize an image format name.
 *
 * Converts image format aliases to a canonical representation
 * in order to simplify format comparisons.
 *
 * Examples:
 * - "jpeg" -> "jpg"
 * - "tiff" -> "tif"
 *
 * The comparison is case-insensitive.
 *
 * @param format Image format name or file extension.
 * @return Normalized image format string.
 */
QString normalizeImageFormat(QString format);

/**
 * @brief Builds a QFileDialog filter string for writable image formats.
 *
 * The supported formats are queried dynamically from Qt image writer plugins
 * using QImageWriter::supportedImageFormats().
 *
 * Example:
 * "PNG (*.png);;JPG (*.jpg);;BMP (*.bmp)"
 *
 * @return A QFileDialog-compatible filter string containing writable image formats.
 */
QString buildWritableImageFilter();

/**
 * @brief Extracts the default file extension from a QFileDialog filter string.
 *
 * Example:
 * "PNG (*.png)" -> "png"
 *
 * @param selectedFilter Selected QFileDialog filter string.
 *
 * @return The extracted extension without the leading dot,
 *         or an empty string if no extension could be detected.
 */
QString defaultExtensionFromFilter(const QString& selectedFilter);

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

/**
 * @brief Returns the list of supported video file extensions.
 *
 * The returned extensions are derived from the multimedia backend
 * capabilities reported by Qt Multimedia for media decoding.
 *
 * Example:
 * "*.avi *.m4v *.mkv *.mov *.mp4 *.webm"
 */
QString supportedVideoExtensions();

/**
 * @brief Builds a QFileDialog filter for supported video files.
 *
 * The filter is generated from the multimedia formats supported
 * by the current Qt Multimedia backend.
 *
 * Example:
 * "Video Files (*.avi *.m4v *.mkv *.mov *.mp4 *.webm)"
 */
QString buildVideoFilter();

} // namespace fluvel::file_utils
