// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"
#include "recording_session.hpp"
#include "recording_types.hpp"
#include "video_export_settings.hpp"
#include "video_exporter.hpp"
#include "video_frame_buffer.hpp"

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QWaitCondition>

#include <atomic>
#include <cstddef>
#include <thread>

namespace fluvel
{

/**
 * @brief Records video frames asynchronously.
 *
 * This class owns a video exporter and queues incoming frames for
 * asynchronous encoding in a dedicated worker thread.
 *
 * The caller controls when recording starts and stops. When recording
 * stops, queued frames are drained before the output video is finalized.
 */
class VideoRecorderWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a video recorder worker.
     */
    VideoRecorderWorker();

    /**
     * @brief Destroys the video recorder worker.
     *
     * Stops the current recording session and waits for the worker thread
     * to terminate.
     */
    ~VideoRecorderWorker();

    /**
     * @brief Starts a new recording session.
     *
     * @param settings Video export settings.
     */
    void start(const VideoExportSettings& settings);

    /**
     * @brief Stops the current recording session.
     *
     * Stops accepting new frames and asynchronously drains the queued
     * frames before finalizing the output video.
     */
    void stop();

    /**
     * @brief Checks whether a recording session is active.
     *
     * A recording session remains active while queued frames are being
     * drained.
     *
     * @return @c true if recording or draining, @c false otherwise.
     */
    [[nodiscard]]
    bool isRecording() const;

    /**
     * @brief Checks whether the recorder is accepting video frames.
     *
     * Frames are accepted only while the recorder is actively recording.
     *
     * @return @c true if the recorder is accepting frames,
     *         @c false otherwise.
     */
    [[nodiscard]]
    bool isAcceptingFrames() const;

    /**
     * @brief Returns the current state of the recorder worker.
     *
     * The returned state reflects the worker's internal recording lifecycle,
     * including frame acceptance and finalization.
     *
     * @return Current recorder worker state.
     */
    [[nodiscard]]
    RecorderState state() const;

    /**
     * @brief Queues a video frame for encoding.
     *
     * @param frame Video frame to encode.
     */
    void addFrame(const VideoFrame& frame);

signals:
    /**
     * @brief Emitted when the recorder state changes.
     *
     * @param state New recorder state.
     */
    void stateChanged(RecorderState state);

    /**
     * @brief Emitted when recorder statistics are updated.
     *
     * @param stats Current recorder statistics.
     */
    void statsChanged(const RecorderStats& stats);

    /**
     * @brief Emitted when the output video has been successfully finalized.
     */
    void recordingFinalized();

    /**
     * @brief Emitted when a non-fatal recording warning occurs.
     *
     * @param message Warning message.
     */
    void warningOccurred(const QString& message);

    /**
     * @brief Emitted when a recording error occurs.
     *
     * @param message Error message.
     */
    void errorOccurred(const QString& message);

private:

    void enqueue(const VideoFrame& frame);
    void processQueue();

    void resetSession();

    void updateStats();

    RecordingSession recordingSession_;
    std::thread workerThread_;

    QMutex mutex_;
    QWaitCondition condition_;

    std::atomic<RecorderState> state_{RecorderState::Stopped};

    std::size_t inputFrameCount_{0};
    std::size_t encodedFrameCount_{0};

    int64_t statsTimestampNs_{0};

    static constexpr int64_t kStatsIntervalNs = 1'000'000'000;

    VideoFrameBuffer frameBuffer_;

    std::optional<int64_t> firstPresentationTimestampNs_;
    std::optional<int64_t> lastPresentationTimestampNs_;
};

} // namespace fluvel