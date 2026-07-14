// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_utils.hpp"

extern "C"
{
#include <libavutil/error.h>
}

#include <utility>

namespace fluvel::ffmpeg_utils
{

const char* containerName(VideoContainer container)
{
    switch (container)
    {
        case VideoContainer::Matroska:
            return "matroska";

        case VideoContainer::Mp4:
            return "mp4";

        case VideoContainer::Avi:
            return "avi";

        case VideoContainer::Mov:
            return "mov";

        case VideoContainer::WebM:
            return "webm";
    }

    std::unreachable();
    return nullptr;
}

QString errorString(int error)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};

    av_strerror(error, buffer, sizeof(buffer));

    return QString::fromUtf8(buffer);
}

} // namespace fluvel::ffmpeg_utils