// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "recording_session.hpp"
#include "frame_clock.hpp"
#include "frame_pipeline.hpp"

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
        const QFileInfo info(settings_.filename);

        segmentDirectory_ = info.absolutePath() + "/" + info.completeBaseName();

        if (!QDir().mkpath(segmentDirectory_))
            return false;

        segmentIndex_ = 1;
        exportSettings.filename = segmentFilename(segmentIndex_);

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

bool RecordingSession::addFrame(const VideoFrame& frame)
{
    if (settings_.recordingMode == RecordingMode::Circular)
    {
        const int64_t timestampNs =
            frame.presentationTimestampNs ? *frame.presentationTimestampNs : FrameClock::nowNs();

        if (!currentSegmentStartNs_)
        {
            currentSegmentStartNs_ = timestampNs;
        }
        else if (timestampNs - *currentSegmentStartNs_ >= segmentDurationNs_)
        {
            if (!rotateSegment())
                return false;

            currentSegmentStartNs_ = timestampNs;
        }
    }

    return exporter_.addFrame(frame);
}

bool RecordingSession::close()
{
    settings_ = {};

    currentSegmentStartNs_.reset();
    segmentDurationNs_ = 0;
    segmentIndex_ = 1;
    segmentDirectory_.clear();

    return exporter_.close();
}

bool RecordingSession::rotateSegment()
{
    assert(settings_.recordingMode == RecordingMode::Circular);

    if (!exporter_.close())
        return false;

    ++segmentIndex_;

    VideoExportSettings exportSettings = settings_;
    exportSettings.filename = segmentFilename(segmentIndex_);

    removeOldSegments();

    return exporter_.open(exportSettings);
}

QString RecordingSession::segmentFilename(int index) const
{
    assert(settings_.recordingMode == RecordingMode::Circular);

    const QFileInfo info(settings_.filename);

    const QString base = info.completeBaseName();
    const QString ext = info.completeSuffix();

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