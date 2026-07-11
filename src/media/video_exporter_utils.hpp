// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"

#include <QString>

namespace fluvel::exporter_utils
{

/**
 * @brief Returns the display name of a video codec.
 *
 * @param codec Video codec.
 * @return Human-readable codec name.
 */
[[nodiscard]]
QString toString(VideoCodec codec);

/**
 * @brief Returns the display name of a video container.
 *
 * @param container Video container.
 * @return Human-readable container name.
 */
[[nodiscard]]
QString toString(VideoContainer container);

/**
 * @brief Ensures that a filename extension matches a video container.
 *
 * If the current extension does not match the selected container,
 * it is replaced with the expected extension.
 *
 * @param filename Output filename to update.
 * @param container Video container.
 */
void ensureExpectedExtension(QString& filename, VideoContainer container);

/**
 * @brief Returns the expected filename extension for a video container.
 *
 * @param container Video container.
 *
 * @return Expected filename extension without the leading dot.
 */
[[nodiscard]]
QString expectedExtension(VideoContainer container);

/**
 * @brief Checks whether a filename extension matches a video container.
 *
 * The comparison is case-insensitive.
 *
 * @param filename Output filename.
 * @param container Video container.
 *
 * @return @c true if the filename extension matches the container,
 *         @c false otherwise.
 */
[[nodiscard]]
bool hasExpectedExtension(const QString& filename, VideoContainer container);

} // namespace fluvel::exporter_utils