// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "recording_session.hpp"
#include "frame_clock.hpp"
#include "frame_pipeline.hpp"
#include "video_exporter_utils.hpp"

#include <QDir>
#include <QFileInfo>

#include <cassert>

namespace fluvel
{

bool RecordingSession::open(const VideoExportSettings& settings)
{
    settings_ = settings;

    VideoExportSettings exportSettings = settings_;

    if (settings_.recordingMode == RecordingMode::Circular)
    {
        const QFileInfo info(settings_.outputPath);

        segmentDirectory_ = info.absolutePath() + "/" + info.completeBaseName();

        if (!QDir().mkpath(segmentDirectory_))
            return false;

        segmentIndex_ = 1;
        exportSettings.outputPath = segmentFilename(segmentIndex_);

        assert(settings_.segmentCount >= 2);
        assert(settings_.retentionTimeMinutes >= 1);

        segmentDurationNs_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                 std::chrono::minutes(settings_.retentionTimeMinutes))
                                 .count() /
                             settings_.segmentCount;
    }

    if (!exporter_.open(exportSettings))
        return false;

    return true;
}

RecordingFailure RecordingSession::lastFailure() const noexcept
{
    return lastFailure_;
}

bool RecordingSession::addFrame(const VideoFrame& frame)
{
    lastFailure_ = RecordingFailure::None;

    if (frame.image.isNull() || frame.image.size().isEmpty())
    {
        lastFailure_ = RecordingFailure::InvalidFrame;
        return false;
    }

    const bool formatChanged =
        frameSize_.isValid() &&
        (frame.image.size() != frameSize_ || frame.image.format() != frameFormat_);

    // In single-file mode the size and format are fixed for the whole file.
    if (formatChanged && settings_.recordingMode == RecordingMode::SingleFile)
    {
        lastFailure_ = RecordingFailure::FrameFormatChanged;
        return false;
    }

    if (settings_.recordingMode == RecordingMode::Circular)
    {
        const int64_t timestampNs =
            frame.presentationTimestampNs ? *frame.presentationTimestampNs : FrameClock::nowNs();

        if (!currentSegmentStartNs_)
        {
            currentSegmentStartNs_ = timestampNs;
        }
        else if (formatChanged || timestampNs - *currentSegmentStartNs_ >= segmentDurationNs_)
        {
            // A change of frame size or format also starts a new segment,
            // since the size and format are fixed for a given output file.
            if (!rotateSegment())
            {
                lastFailure_ = RecordingFailure::SegmentRotationFailed;
                return false;
            }

            currentSegmentStartNs_ = timestampNs;
        }
    }

    if (!exporter_.addFrame(frame))
    {
        lastFailure_ = RecordingFailure::EncodingFailed;
        return false;
    }

    if (!frameSize_.isValid() || settings_.recordingMode == RecordingMode::Circular)
    {
        frameSize_ = frame.image.size();
        frameFormat_ = frame.image.format();
    }

    return true;
}

bool RecordingSession::close()
{
    // Remember what has to be converted once the files are closed.
    pendingConversion_ = settings_.convertToStandardMp4 && settings_.container == VideoContainer::Mp4;
    conversionSettings_ = settings_;
    conversionDirectory_ = segmentDirectory_;

    conversionFailed_ = false;

    settings_ = {};

    frameSize_ = QSize();
    frameFormat_ = QImage::Format_Invalid;

    currentSegmentStartNs_.reset();
    segmentDurationNs_ = 0;
    segmentIndex_ = 1;
    segmentDirectory_.clear();

    const bool success = exporter_.close();

    if (!success)
        pendingConversion_ = false;

    return success;
}

bool RecordingSession::hasPendingConversion() const noexcept
{
    return pendingConversion_;
}

void RecordingSession::convertFiles()
{
    // The conversion is done once, at the very end: all the files are final.
    if (!pendingConversion_)
        return;

    pendingConversion_ = false;

    const VideoExportSettings settings = conversionSettings_;
    const QString directory = conversionDirectory_;

    conversionSettings_ = {};
    conversionDirectory_.clear();

    QStringList files;

    if (settings.recordingMode == RecordingMode::SingleFile)
    {
        files << settings.outputPath;
    }
    else
    {
        const QDir dir(directory);

        const QStringList names = dir.entryList({"*.mp4"}, QDir::Files, QDir::Name);

        for (const QString& name : names)
            files << dir.filePath(name);
    }

    // A failed file keeps its fragmented original; the others are still converted.
    for (const QString& file : files)
    {
        if (!VideoExporter::convertToStandardMp4(file))
            conversionFailed_ = true;
    }
}

bool RecordingSession::conversionFailed() const noexcept
{
    return conversionFailed_;
}

bool RecordingSession::rotateSegment()
{
    assert(settings_.recordingMode == RecordingMode::Circular);

    if (!exporter_.close())
        return false;

    ++segmentIndex_;

    VideoExportSettings exportSettings = settings_;
    exportSettings.outputPath = segmentFilename(segmentIndex_);

    removeOldSegments();

    return exporter_.open(exportSettings);
}

QString RecordingSession::segmentFilename(int index) const
{
    assert(settings_.recordingMode == RecordingMode::Circular);

    const QFileInfo info(settings_.outputPath);

    const QString base = info.fileName();
    const QString ext = exporter_utils::expectedExtension(settings_.container);

    return QString("%1/%2_%3.%4")
        .arg(segmentDirectory_)
        .arg(base)
        .arg(index, 6, 10, QLatin1Char('0'))
        .arg(ext);
}

void RecordingSession::removeOldSegments()
{
    assert(settings_.recordingMode == RecordingMode::Circular);

    // Keep only the most recent segmentCount segments.
    const int oldest = segmentIndex_ - settings_.segmentCount;

    if (oldest > 0)
        QFile::remove(segmentFilename(oldest));
}

} // namespace fluvel