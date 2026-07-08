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
inline QString toString(VideoCodec codec)
{
    switch (codec)
    {
        case VideoCodec::FFV1:
            return "FFV1";
        case VideoCodec::MPEG4Part2:
            return "MPEG-4 Part 2";
        case VideoCodec::H264:
            return "H.264";
        case VideoCodec::H265:
            return "H.265";
        case VideoCodec::VP9:
            return "VP9";
        case VideoCodec::AV1:
            return "AV1";
    }

    std::unreachable();
    return {};
}

} // namespace fluvel::exporter_utils