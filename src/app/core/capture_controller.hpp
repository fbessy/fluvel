// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"

#ifdef FLUVEL_USE_FFMPEG
#include "video_export_settings.hpp"
#include "video_recorder_worker.hpp"
#endif

#include <QObject>

namespace fluvel
{

/**
 * @brief Handles image capture and video recording.
 *
 * The controller owns the recording worker and provides the API for
 * snapshot and recording operations.
 *
 * Its responsibilities include:
 * - storing the latest frame for snapshot capture,
 * - recording videos,
 * - forwarding video frames to the recorder,
 * - exposing recording state and statistics.
 *
 * It is intentionally independent from VideoController so that it can be
 * reused by different video sources (camera, media player, RTSP, etc.).
 */
class CaptureController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a capture controller.
     *
     * @param parent Parent QObject.
     */
    explicit CaptureController(QObject* parent = nullptr);

    ~CaptureController() override = default;

    /**
     * @brief Submits a new video frame.
     *
     * The frame is stored for snapshot capture and forwarded to the recorder
     * when recording is active.
     *
     * @param frame Video frame.
     */
    void submitFrame(const fluvel::VideoFrame& frame);

    /**
     * @brief Sets the streaming state.
     *
     * @param streaming True if a stream is currently active.
     */
    void setStreaming(bool streaming);

    /**
     * @brief Returns whether a stream is active.
     */
    [[nodiscard]]
    bool isStreaming() const noexcept;

    /**
     * @brief Requests a snapshot of the current displayed frame.
     *
     * Emits snapshotRequested(). The associated window is responsible for
     * providing the current display frame to the controller.
     */
    void requestSnapshot();

    /**
     * @brief Saves a snapshot image.
     *
     * The image is saved according to the persistent snapshot preferences.
     *
     * @param image Image to save.
     */
    void saveSnapshot(const QImage& image);

#ifdef FLUVEL_USE_FFMPEG

    /**
     * @brief Starts video recording.
     */
    void startRecording();

    /**
     * @brief Stops video recording.
     */
    void stopRecording();

    /**
     * @brief Returns whether a recording is active or being finalized.
     */
    [[nodiscard]]
    bool isRecording() const noexcept;

    /**
     * @brief Returns whether the recorder is currently accepting video frames.
     *
     * This returns @c true only while recording is active and new frames can
     * be submitted to the recorder. It returns @c false while the recorder is
     * stopped or finalizing the recording.
     */
    [[nodiscard]]
    bool isAcceptingFrames() const noexcept;

    /**
     * @brief Returns the recorder state.
     */
    [[nodiscard]]
    RecorderState recordingState() const noexcept;

#endif

signals:

#ifdef FLUVEL_USE_FFMPEG

    /**
     * @brief Emitted whenever the recorder state changes.
     */
    void recordingStateChanged(RecorderState state);

    /**
     * @brief Emitted periodically with recorder statistics.
     */
    void recordingStatsChanged(const RecorderStats& stats);

    /**
     * @brief Emitted when recording starts.
     */
    void recordingStarted(const QString& filename);

    /**
     * @brief Emitted when recording has been finalized.
     */
    void recordingFinalized(const QString& filename);

    /**
     * @brief Emitted when a non-fatal recording warning occurs.
     */
    void recordingWarning(const QString& message);

    /**
     * @brief Emitted when recording fails.
     */
    void recordingError(const QString& message);

#endif

    /**
     * @brief Emitted when a snapshot is requested.
     *
     * The window displaying the current frame should respond by providing
     * the image to save through saveSnapshot().
     */
    void snapshotRequested();

    /**
     * @brief Emitted after a snapshot has been saved.
     */
    void snapshotSaved(const QString& filename);

    /**
     * @brief Emitted when snapshot creation fails.
     */
    void snapshotError(const QString& message);

    /**
     * @brief Emitted when the streaming state changes.
     */
    void streamingChanged(bool streaming);

private:
#ifdef FLUVEL_USE_FFMPEG

    /**
     * @brief Forwards recorder state changes.
     *
     * @param state Current recorder state.
     */
    void onRecordingStateChanged(RecorderState state);

    VideoExportSettings recordingSettings_{};
    VideoRecorderWorker recorder_;

#endif

    bool streaming_{false};
};

} // namespace fluvel