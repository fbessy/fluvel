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
 * @brief Resolves the effective video export settings.
 *
 * Applies the selected export profile and ensures that the output
 * filename extension matches the resolved container when using a
 * predefined profile.
 *
 * Custom settings are preserved and the filename is left unchanged.
 *
 * @param settings Requested video export settings.
 *
 * @return Resolved video export settings.
 */
[[nodiscard]]
VideoExportSettings resolveSettings(const VideoExportSettings& settings);

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

/**
 * @brief Returns the preferred container for a video codec.
 *
 * @param codec Video codec.
 * @return Preferred video container.
 */
VideoContainer preferredContainer(VideoCodec codec);

} // namespace fluvel::exporter_utils