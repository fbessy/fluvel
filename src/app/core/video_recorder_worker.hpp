// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"
#include "video_export_settings.hpp"
#include "video_exporter.hpp"
#include "video_types.hpp"

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
    enum class EnqueueStatus
    {
        Success,
        MemoryWarning,
        MemoryLimitExceeded
    };

    void enqueue(const VideoFrame& frame);
    void processQueue();

    void resetSession();

    static std::size_t frameSize(const QImage& image);

    VideoExporter exporter_;
    std::thread workerThread_;
    QQueue<VideoFrame> queue_;

    QMutex mutex_;
    QWaitCondition condition_;

    std::atomic<RecorderState> state_{RecorderState::Stopped};

    std::size_t queuedBytes_{0};

    bool memoryWarningEmitted_{false};

    static constexpr std::size_t kWarningMemoryBytes = 1200ull * 1024 * 1024; // 1200 MiB

    static constexpr std::size_t kMaxMemoryBytes = 2000ull * 1024 * 1024; // 2000 MiB
};

} // namespace fluvel