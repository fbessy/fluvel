// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include "frame_pipeline.hpp"
#include "frame_stats_collector.hpp"
#include "video_active_contour_thread.hpp"
#include "video_types.hpp"

#include <QAudioOutput>
#include <QByteArray>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaPlayer>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVideoSink>
#include <QtCore/qglobal.h>

// #define FLUVEL_SIMULATE_STARTUP_TIMEOUT
// #define FLUVEL_SIMULATE_STREAM_LOSS

namespace fluvel
{

/**
 * @brief High-level controller for video streaming and processing.
 *
 * VideoController manages the complete lifecycle of a video source,
 * from source initialization and frame reception to active contour
 * processing and UI updates.
 *
 * Supported sources include:
 * - camera devices
 * - network video streams
 * - local video files
 *
 * Responsibilities include:
 * - source initialization and startup
 * - frame reception through Qt Multimedia
 * - dispatching frames to the processing thread
 * - monitoring stream health (startup timeout, watchdog, stream loss)
 * - publishing processed frames, contours and diagnostics to the UI
 *
 * This class acts as the bridge between the Qt Multimedia subsystem
 * and the image-processing backend.
 *
 * @note This class is tied to the Qt event loop and is intended to run
 * in the main thread.
 */
class VideoController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a video controller with initial session settings.
     */
    explicit VideoController(const VideoSessionSettings& session, QObject* parent = nullptr);

    ~VideoController() override;

    /**
     * @brief Starts streaming from the specified source.
     */
    void start(const SourceConfig& sourceConfig);

    /**
     * @brief Stop the current stream.
     */
    void stop();

    /**
     * @brief Check whether streaming is active.
     */
    bool isStreaming() const;

    /**
     * @brief Check the streaming state.
     */
    StreamingState streamingState() const;

    /**
     * @brief Getter for active source info.
     */
    SourceInfo activeSource() const;

    /**
     * @brief List available video input devices.
     */
    QList<QCameraDevice> videoInputs() const;

    /**
     * @brief Update processing configuration for video session.
     */
    void onVideoSettingsChanged(const VideoSessionSettings& session);

    /**
     * @brief Update display configuration for video session.
     */
    void onVideoDisplaySettingsChanged(const DisplayConfig& display);

    /**
     * @brief Notify controller that a frame has been displayed.
     *
     * Used for timing/latency measurements.
     */
    void onFrameDisplayed(const FrameTimestamps& ts);

    /**
     * @brief Seek to a position in the current media.
     *
     * @param posMs Target position in milliseconds.
     */
    void seek(qint64 posMs);

    /**
     * @brief Returns the current audio volume.
     *      * @return Volume in the range [0.0, 1.0].
     */
    float volume() const;

    /**
     * @brief Sets the audio volume.
     *      * @param volume Volume in the range [0.0, 1.0].
     */
    void setVolume(float volume);

    /**
     * @brief Check whether media playback is currently paused.
     *
     * @return True if playback is paused, false otherwise.
     */
    bool isPaused() const;

    /**
     * @brief Pause media playback.
     */
    void pause();

    /**
     * @brief Resume media playback.
     */
    void resume();

signals:
    /// Emitted when available video inputs change.
    void videoInputsChanged(const QList<QCameraDevice>& devices);

    /**
     * @brief Emitted when source startup begins.
     *
     * The controller entered the starting phase and is waiting
     * for the first valid frame before entering streaming mode.
     */
    void streamingStarting();

    /**
     * @brief Emitted when streaming successfully starts.
     *
     * A valid frame was received and the controller entered
     * the streaming state.
     */
    void streamingStarted(const fluvel::StreamingInfo& info);

    /**
     * @brief Emitted when streaming stops.
     *
     * The controller released the active stream and entered
     * the stopped state.
     */
    void streamingStopped();

    /// Emitted on camera error.
    void cameraError(const fluvel::CameraErrorInfo& errorInfo);

    /// Emitted on media player error.
    void mediaPlayerError(const fluvel::MediaPlayerErrorInfo& errorInfo);

    /// Emitted when startup timeout is reached.
    void startupTimeout(const fluvel::SourceInfo& sourceInfo, double timeoutSec);

    /// Emitted when stream loss is detected.
    void streamingLost(const fluvel::StreamingInfo& streamingInfo, double frameAgeSec);

    /// Emitted with updated textual statistics.
    void textStatsUpdated(const QString& textStats);

    /// Emitted with processed frame and contour overlay.
    void imageAndContourUpdated(const fluvel::UiFrame& uiFrame);

    /// Emitted when downscale parameters change.
    void downscaleChanged(const fluvel::DownscaleParams& downscaleParams);

    /**
     * @brief Emitted when the playback position changes.
     *
     * @param positionMs Current playback position in milliseconds.
     */
    void playbackPositionChanged(qint64 positionMs);

    /**
     * @brief Emitted when media information changes.
     *
     * @param info Updated media information.
     */
    void mediaInfoChanged(const MediaInfo& info);

    /**
     * @brief Emitted when media playback is paused or resumed.
     *
     * @param paused Current pause state.
     */
    void pausedChanged(bool paused);

private:
    /**
     * @brief Start streaming using a specific device.
     */
    void start(const QByteArray& deviceId);

    /**
     * @brief Start streaming with a specific device and format.
     */
    void start(const QByteArray& deviceId, const QCameraFormat& format);

    /**
     * @brief Starts streaming from a media URL.
     */
    void start(const QUrl& url);

    /// Handle updates in available video inputs.
    void onVideoInputsChanged();

    /// Handle unplug of the currently active device.
    void handleActiveDeviceUnplug(const QList<QCameraDevice>& devices);

    /// Handle camera error callback.
    void onCameraError(QCamera::Error error, const QString& errorString);

    /// Handle media player error callback.
    void onMediaPlayerError(QMediaPlayer::Error error, const QString& errorString);

    /// Called when a new video frame is received from the active source.
    void onFrameReceived(const QVideoFrame& frame);

    /// Called when frame processing has completed.
    void onFrameProcessed(quint64 contourSize);

    /// Called when a display-ready frame is available.
    void onDisplayFrameReady(const DisplayFrame& displayFrame);

    /// Triggered when startup timeout is reached.
    void onStartupTimeout();

    /**
     * @brief Handle media player status changes.
     *
     * @param status New media status.
     */
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

    /**
     * @brief Handle media metadata updates.
     */
    void onMetaDataChanged();

    /// Periodic watchdog to detect stream loss.
    void checkWatchdog();

    /// Update internal diagnostics.
    void updateDiagnostics();

    /**
     * @brief Refresh media information from the active source.
     */
    void updateMediaInfo();

    /**
     * @brief Determine whether a media title is suitable for display.
     *
     * Filters out empty or non-informative titles returned by multimedia
     * backends.
     *
     * @param title Media title to evaluate.
     * @return True if the title is considered useful.
     */
    static bool isUsefulMediaTitle(const QString& title);

    /// Whether to use an optimized camera format when available.
    bool useOptimizedFormat_{true};

    /// Qt camera object.
    QCamera* camera_{nullptr};

    /// Qt capture session.
    QMediaCaptureSession captureSession_;

    QMediaPlayer mediaPlayer_;
    QAudioOutput audioOutput_;

    /// Video sink receiving frames.
    QVideoSink videoSink_;

    /// Processing thread running active contour.
    VideoActiveContourThread activeContourThread_;

    /// Frame statistics and diagnostics view.
    FrameStatsCollector frameStats_;

    // --- Timing configuration ---

    static constexpr int kStartupTimeoutMs{7'000};                // 7 sec
    static constexpr int64_t kStreamLossTimeoutNs{2'000'000'000}; // 2 sec
    static constexpr int kWatchdogPeriodMs{200};                  // 0.2 sec
    static constexpr int kDiagnosticsPeriodMs{500};               // 0.5 sec

    /// Timer used to detect startup timeout.
    QTimer startupTimer_;

    /// Timer used for stream watchdog.
    QTimer watchdogTimer_;

    /// Timer used for periodic diagnostics updates.
    QTimer diagnosticsTimer_;

    /// Current streaming state.
    StreamingState state_{StreamingState::Stopped};

    /// Information about the source being opened or streamed.
    SourceInfo startupInfo_{};

    /// Runtime information about the active stream.
    StreamingInfo streamingInfo_{};

    /// Information about the currently loaded media.
    MediaInfo mediaInfo_{};

    //! Monotonic timestamp (ns) of the last valid frame, used for stream loss detection.
    int64_t lastValidFrameTsNs_{0};

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    int testFrameCounter_{0};
#endif

    /// Whether the watchdog is allowed to report stream loss.
    bool watchdogArmed_{true};
};

} // namespace fluvel
