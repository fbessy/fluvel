// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QString>

namespace fluvel::ffmpeg_utils
{

/**
 * @brief Converts a fragmented MP4 file into a standard MP4 file.
 *
 * The streams are copied without re-encoding. The result is first written
 * to a temporary file next to the original, then replaces it. If anything
 * fails, the original file is left untouched.
 *
 * @param path Path of the MP4 file to convert.
 * @return @c true on success, @c false otherwise.
 */
[[nodiscard]]
bool remuxToStandardMp4(const QString& path);

} // namespace fluvel::ffmpeg_utils
