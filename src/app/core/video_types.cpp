// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_types.hpp"

#include "camera_format_utils.hpp"

namespace fluvel
{

bool SourceInfo::matches(const SourceConfig& config) const
{
    if (type != config.type)
        return false;

    switch (type)
    {
        case SourceType::Camera:
            return deviceId == config.cameraId &&
                   camera_utils::isSameCameraFormat(deviceFormat, config.cameraFormat);

        case SourceType::Media:
            return sourceUrl == config.url;

        case SourceType::None:
            return true;
    }

    return false;
}

} // namespace fluvel