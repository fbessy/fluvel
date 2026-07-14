// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_exporter_utils.hpp"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>

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

/**
 * @brief Applies an export profile.
 *
 * Converts high-level export profiles into explicit codec
 * and container selections.
 *
 * @param settings Export settings to update.
 */
static void applyExportProfile(VideoExportSettings& settings)
{
    switch (settings.profile)
    {
        case ExportProfile::Archive:

            settings.codec = VideoCodec::FFV1;
            settings.container = VideoContainer::Matroska;
            break;

        case ExportProfile::Compatible:

            settings.codec = VideoCodec::H264;
            settings.container = VideoContainer::Mp4;
            break;

        case ExportProfile::Balanced:

            settings.codec = VideoCodec::H265;
            settings.container = VideoContainer::Mp4;
            break;

        case ExportProfile::Efficient:

            settings.codec = VideoCodec::AV1;
            settings.container = VideoContainer::Mp4;
            break;

        case ExportProfile::Custom:

            if (settings.codec == VideoCodec::FFV1 &&
                settings.container != VideoContainer::Matroska)
            {
                qWarning() << "FFV1 is typically stored in a Matroska container.";
            }

            if (settings.codec == VideoCodec::VP9 && settings.container != VideoContainer::WebM)
            {
                qWarning() << "VP9 is typically stored in a WebM container.";
            }

            if ((settings.codec == VideoCodec::H264 || settings.codec == VideoCodec::H265) &&
                settings.container != VideoContainer::Mp4)
            {
                qInfo() << exporter_utils::toString(settings.codec)
                        << "is commonly stored in an MP4 container.";
            }

            // No override.
            break;
    }
}

/**
 * @brief Ensures that a filename extension matches a video container.
 *
 * If the current extension does not match the selected container,
 * it is replaced with the expected extension.
 *
 * @param filename Output filename to update.
 * @param container Video container.
 */
static void ensureExpectedExtension(QString& filename, VideoContainer container)
{
    if (hasExpectedExtension(filename, container))
        return;

    const QFileInfo fileInfo(filename);

    filename =
        fileInfo.dir().filePath(fileInfo.completeBaseName() + "." + expectedExtension(container));
}

VideoExportSettings resolveSettings(const VideoExportSettings& settings)
{
    VideoExportSettings resolved = settings;

    applyExportProfile(resolved);

    if (resolved.filename.isEmpty())
    {
        const QString filename = "video." + expectedExtension(resolved.container);

        resolved.filename = QDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
                                .filePath(filename);
    }
    else if (resolved.profile != ExportProfile::Custom)
    {
        ensureExpectedExtension(resolved.filename, resolved.container);
    }

    return resolved;
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