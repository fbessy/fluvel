// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"

namespace fluvel::ffmpeg_utils
{

/**
 * @brief Returns the FFmpeg muxer name for a video container.
 *
 * @param container Video container.
 * @return FFmpeg muxer name.
 */
[[nodiscard]]
const char* containerName(VideoContainer container);

/**
 * @brief Returns the description of an FFmpeg error code.
 *
 * @param error FFmpeg error code.
 * @return Human-readable error description.
 */
[[nodiscard]]
QString errorString(int error);

} // namespace fluvel::ffmpeg_utils