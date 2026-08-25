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
            return camera.matches(config.camera);

        case SourceType::Media:
            return media.matches(config.media);

        case SourceType::None:
            return true;
    }

    return false;
}

bool CameraInfo::matches(const CameraConfig& config) const
{
    return deviceId == config.deviceId &&
           camera_utils::isSameCameraFormat(deviceFormat, config.deviceFormat);
}

bool MediaSourceInfo::matches(const MediaSourceConfig& config) const
{
    return sourceUrl == config.sourceUrl;
}

} // namespace fluvel
