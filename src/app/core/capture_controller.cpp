// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "capture_controller.hpp"

#include "application_settings.hpp"
#include "file_utils.hpp"

#ifdef FLUVEL_USE_FFMPEG
#include "video_exporter_utils.hpp"
#endif

namespace fluvel
{

CaptureController::CaptureController(QObject* parent)
    : QObject(parent)
{
#ifdef FLUVEL_USE_FFMPEG

    connect(&recorder_, &VideoRecorderWorker::stateChanged, this,
            &CaptureController::onRecordingStateChanged);

    connect(&recorder_, &VideoRecorderWorker::statsChanged, this,
            &CaptureController::recordingStatsChanged);

    connect(&recorder_, &VideoRecorderWorker::warningOccurred, this,
            &CaptureController::recordingWarning);

    connect(&recorder_, &VideoRecorderWorker::errorOccurred, this,
            &CaptureController::recordingError);

    connect(&recorder_, &VideoRecorderWorker::recordingFinalized, this,
            [this]()
            {
                emit recordingFinalized(recordingSettings_.outputPath);
            });

#endif

    connect(&snapshotWorker_, &SnapshotWorker::snapshotSaved, this,
            &CaptureController::snapshotSaved);

    connect(&snapshotWorker_, &SnapshotWorker::snapshotError, this,
            &CaptureController::snapshotError);
}

void CaptureController::submitFrame(const fluvel::VideoFrame& frame)
{
#ifdef FLUVEL_USE_FFMPEG

    if (recorder_.isAcceptingFrames())
        recorder_.addFrame(frame);

#endif
}

void CaptureController::setStreaming(bool streaming)
{
    if (streaming_ == streaming)
        return;

    streaming_ = streaming;

    emit streamingChanged(streaming_);
}

bool CaptureController::isStreaming() const noexcept
{
    return streaming_;
}

#ifdef FLUVEL_USE_FFMPEG

void CaptureController::startRecording()
{
    const auto& preferences = ApplicationSettings::instance().videoRecordingPreferences();

    VideoExportSettings settings;

    settings.profile = ExportProfile::Custom;
    settings.codec = preferences.preferredCodec;
    settings.container = exporter_utils::preferredContainer(settings.codec);

    const QString extension = preferences.recordingMode == RecordingMode::SingleFile
                                  ? exporter_utils::expectedExtension(settings.container)
                                  : QString();

    settings.outputPath = file_utils::buildOutputFileName(
        preferences.directory, preferences.baseName, extension, preferences.appendTimestamp);

    settings.recordingMode = preferences.recordingMode;
    settings.retentionTimeMinutes = preferences.retentionTimeMinutes;
    settings.segmentCount = preferences.segmentCount;

    settings.bufferSettings = ApplicationSettings::instance().recordingBufferSettings();

    recordingSettings_ = exporter_utils::resolveSettings(settings);

    recorder_.start(recordingSettings_);
}

void CaptureController::stopRecording()
{
    recorder_.stop();
}

bool CaptureController::isRecording() const noexcept
{
    return recorder_.isRecording();
}

bool CaptureController::isAcceptingFrames() const noexcept
{
    return recorder_.isAcceptingFrames();
}

RecorderState CaptureController::recordingState() const noexcept
{
    return recorder_.state();
}

void CaptureController::onRecordingStateChanged(RecorderState state)
{
    emit recordingStateChanged(state);

    if (state == RecorderState::Recording)
        emit recordingStarted(recordingSettings_.outputPath);
}

#endif

void CaptureController::requestSnapshot()
{
    emit snapshotRequested();
}

void CaptureController::saveSnapshot(const QImage& image)
{
    if (image.isNull())
    {
        emit snapshotError(tr("No frame available."));
        return;
    }

    if (!snapshotWorker_.enqueue(image))
    {
        emit snapshotError(tr("Snapshot queue is full."));
    }
}

} // namespace fluvel