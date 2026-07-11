// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_exporter_utils.hpp"

#include <QDir>
#include <QFileInfo>

namespace fluvel::exporter_utils
{

QString toString(VideoCodec codec)
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

QString toString(VideoContainer container)
{
    switch (container)
    {
        case VideoContainer::Matroska:
            return "Matroska";
        case VideoContainer::Mp4:
            return "MP4";
        case VideoContainer::WebM:
            return "WebM";
        case VideoContainer::Mov:
            return "QuickTime";
        case VideoContainer::Avi:
            return "AVI";
    }

    std::unreachable();
}

void ensureExpectedExtension(QString& filename, VideoContainer container)
{
    if (hasExpectedExtension(filename, container))
        return;

    const QFileInfo fileInfo(filename);

    filename =
        fileInfo.dir().filePath(fileInfo.completeBaseName() + "." + expectedExtension(container));
}

QString expectedExtension(VideoContainer container)
{
    switch (container)
    {
        case VideoContainer::Matroska:
            return "mkv";

        case VideoContainer::Mp4:
            return "mp4";

        case VideoContainer::WebM:
            return "webm";

        case VideoContainer::Mov:
            return "mov";

        case VideoContainer::Avi:
            return "avi";
    }

    std::unreachable();
}

bool hasExpectedExtension(const QString& filename, VideoContainer container)
{
    return QFileInfo(filename).suffix().compare(expectedExtension(container),
                                                Qt::CaseInsensitive) == 0;
}

} // namespace fluvel::exporter_utils