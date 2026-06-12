// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include "frame_data.hpp"
#include "frame_stats_view.hpp"
#include "video_active_contour_thread.hpp"
#include "video_types.hpp"

#include <QByteArray>
#include <QCamera>
#include <QMediaPlayer>
#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>

// #define FLUVEL_SIMULATE_STARTUP_TIMEOUT
// #define FLUVEL_SIMULATE_STREAM_LOSS

class QMediaCaptureSession;
class QVideoSink;
class QTimer;
class QAudioOutput;

namespace fluvel
{

struct StreamingInfo;

/**
 * @brief High-level controller for camera streaming and processing.
 *
 * VideoController manages the complete lifecycle of a video stream,
 * from camera initialization and frame acquisition to active contour
 * processing and UI updates.
 *
 * Responsibilities include:
 * - device selection and camera startup
 * - frame acquisition through Qt Multimedia
 * - dispatching frames to the processing thread
 * - monitoring stream health (startup timeout, watchdog, stream loss)
 * - publishing processed frames, contours and diagnostics to the UI
 *
 * This class acts as the bridge between the Qt Multimedia subsystem
 * (QCamera, QVideoSink) and the image-processing backend.
 *
 * @note This class is tied to the Qt event loop and is intended to run
 * in the main thread.
 */
class VideoController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a camera controller with initial session settings.
     */
    explicit VideoController(const VideoSessionSettings& session, QObject* parent = nullptr);

    /**
     * @brief Destroy the controller and release resources.
     */
    ~VideoController() override;

    /**
     * @brief Start streaming using a specific device.
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

signals:
    /// Emitted when available video inputs change.
    void videoInputsChanged(const QList<QCameraDevice>& devices);

    /**
     * @brief Emitted when camera startup begins.
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
    void cameraError(const QByteArray& deviceId, QCamera::Error error, const QString& errorString);

    /// Emitted when startup timeout is reached.
    void startupTimeout(const SourceInfo& sourceInfo, double timeoutSec);

    /// Emitted when stream loss is detected.
    void streamingLost(const StreamingInfo& streamingInfo, double frameAgeSec);

    /// Emitted with updated textual statistics.
    void textStatsUpdated(const QString& textStats);

    /// Emitted with processed frame and contour overlay.
    void imageAndContourUpdated(const fluvel::UiFrame& uiFrame);

    /// Emitted when downscale parameters change.
    void downscaleChanged(const fluvel::DownscaleParams& downscaleParams);

private:
    /**
     * @brief Start streaming using a specific device.
     */
    void start(const QByteArray& deviceId);

    /**
     * @brief Start streaming with a specific device and format.
     */
    void start(const QByteArray& deviceId, const QCameraFormat& format);

    void start(const QUrl& url);

    /// Handle updates in available video inputs.
    void onVideoInputsChanged();

    /// Handle unplug of the currently active device.
    void handleActiveDeviceUnplug(const QList<QCameraDevice>& devices);

    /// Handle camera error callback.
    void onCameraError(QCamera::Error error, const QString& errorString);

    /// Handle incoming video frame from Qt.
    void onCapturedFrame(const QVideoFrame& frame);

    /// Called when processing thread produces contour data.
    void onFrameProcessed(quint64 contourSize);

    /// Called when a display-ready frame is available.
    void onDisplayFrameReady(const DisplayFrame& displayFrame);

    /// Triggered when startup timeout is reached.
    void onStartupTimeout();

    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

    /// Periodic watchdog to detect stream loss.
    void checkWatchdog();

    /// Update internal diagnostics.
    void updateDiagnostics();

    static QString shortSourceName(const QUrl& url);

    /// Whether to use an optimized camera format when available.
    bool useOptimizedFormat_{true};

    /// Qt camera object.
    QCamera* camera_ = nullptr;

    /// Qt capture session.
    QMediaCaptureSession* captureSession_ = nullptr;

    QMediaPlayer* mediaPlayer_ = nullptr;
    QAudioOutput* audioOutput_ = nullptr;

    /// Video sink receiving frames.
    QVideoSink* videoSink_ = nullptr;

    /// Processing thread running active contour.
    VideoActiveContourThread activeContourThread_;

    /// Frame statistics and diagnostics view.
    FrameStatsView frameStats_;

    // --- Timing configuration ---

    static constexpr int kStartupTimeoutMs{7'000};                // 7 sec
    static constexpr int64_t kStreamLossTimeoutNs{2'000'000'000}; // 2 sec
    static constexpr int kWatchdogPeriodMs{200};                  // 0.2 sec
    static constexpr int kDiagnosticsPeriodMs{500};               // 0.5 sec

    /// Timer used to detect startup timeout.
    QTimer* startupTimer_ = nullptr;

    /// Timer used for stream watchdog.
    QTimer* watchdogTimer_ = nullptr;

    /// Timer used for periodic diagnostics updates.
    QTimer* diagnosticsTimer_ = nullptr;

    /// Current streaming state.
    StreamingState state_ = StreamingState::Stopped;

    /// Current source info.
    SourceInfo sourceInfo_;

    /// Current streaming info.
    StreamingInfo streamingInfo_;

    //! Monotonic timestamp (ns) of the last valid frame, used for stream loss detection.
    int64_t lastValidFrameTsNs_{0};

    //! Timeout used to detect loss of video stream.

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    int testFrameCounter_{0};
#endif
};

} // namespace fluvel
