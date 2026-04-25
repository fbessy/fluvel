// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file camera_controller.hpp
 * @brief Camera capture and processing controller.
 *
 * This component manages the full lifecycle of a video stream:
 * - device selection and camera startup
 * - frame acquisition from Qt multimedia
 * - processing via active contour pipeline
 * - UI updates and diagnostics
 *
 * It acts as the bridge between Qt multimedia (QCamera, QVideoSink)
 * and the image processing backend.
 */

#pragma once

#include "application_settings_types.hpp"
#include "frame_data.hpp"
#include "frame_stats_view.hpp"
#include "video_active_contour_thread.hpp"

#include <QByteArray>
#include <QCamera>
#include <QObject>
#include <QtTypes>

// #define FLUVEL_SIMULATE_STARTUP_TIMEOUT
// #define FLUVEL_SIMULATE_STREAM_LOSS

class QMediaCaptureSession;
class QVideoSink;
class QTimer;

namespace fluvel_app
{

/**
 * @brief Current streaming state.
 */
enum class StreamingState
{
    Stopped,  ///< No active stream
    Starting, ///< Camera initialization in progress
    Streaming ///< Frames are being received
};

/**
 * @brief Information about an active video stream.
 *
 * Contains the selected device and format used for streaming.
 */
struct StreamingInfo
{
    QByteArray deviceId;  ///< Unique device identifier
    QString description;  ///< Human-readable device description
    QCameraFormat format; ///< Active camera format
};

/**
 * @brief High-level controller for camera streaming and processing.
 *
 * Responsibilities:
 * - Manage camera lifecycle (start/stop)
 * - Receive frames from Qt (QVideoSink)
 * - Dispatch frames to processing thread
 * - Monitor stream health (timeouts, watchdog)
 * - Emit UI-ready frames and statistics
 *
 * @note This class is tied to Qt event loop and runs in the main thread.
 */
class CameraController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a camera controller with initial session settings.
     */
    explicit CameraController(const VideoSessionSettings& session, QObject* parent = nullptr);

    /**
     * @brief Destroy the controller and release resources.
     */
    ~CameraController() override;

    /**
     * @brief Start streaming using a specific device.
     */
    void start(const QByteArray& deviceId);

    /**
     * @brief Start streaming with a specific device and format.
     */
    void start(const QByteArray& deviceId, const QCameraFormat& format);

    /**
     * @brief Stop the current stream.
     */
    void stop();

    /**
     * @brief Check whether streaming is active.
     */
    bool isStreaming() const;

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
     *      * Used for timing/latency measurements.
     */
    void onFrameDisplayed(const FrameTimestamps& ts);

signals:
    /// Emitted when available video inputs change.
    void videoInputsChanged(const QList<QCameraDevice>& devices);

    /// Emitted when streaming successfully starts.
    void streamingStarted(const fluvel_app::StreamingInfo& info);

    /// Emitted when streaming stops.
    void streamingStopped();

    /// Emitted on camera error.
    void cameraError(const QByteArray& deviceId, QCamera::Error error, const QString& errorString);

    /// Emitted when startup timeout is reached.
    void startupTimeout(const QByteArray& deviceId, double timeoutSec);

    /// Emitted when stream loss is detected.
    void streamingLost(const QByteArray& deviceId, double frameAgeSec);

    /// Emitted with updated textual statistics.
    void textStatsUpdated(const QString& textStats);

    /// Emitted with processed frame and contour overlay.
    void imageAndContourUpdated(const fluvel_app::UiFrame& uiFrame);

    /// Emitted when downscale parameters change.
    void downscaleChanged(const fluvel_app::DownscaleParams& downscaleParams);

private:
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

    /// Periodic watchdog to detect stream loss.
    void checkWatchdog();

    /// Update internal diagnostics.
    void updateDiagnostics();

    /// Whether to use an optimized camera format when available.
    bool useOptimizedFormat_{true};

    /// Qt camera object.
    QCamera* camera_ = nullptr;

    /// Qt capture session.
    QMediaCaptureSession* captureSession_ = nullptr;

    /// Video sink receiving frames.
    QVideoSink* videoSink_ = nullptr;

    /// Processing thread running active contour.
    VideoActiveContourThread activeContourThread_;

    /// Frame statistics and diagnostics view.
    FrameStatsView frameStats_;

    // --- Timing configuration ---

    static constexpr int kStartupTimeoutMs = 7'000;               // 7 sec
    static constexpr int64_t kStreamLossTimeoutNs = 2'000'000'000; // 2 sec
    static constexpr int kWatchdogPeriodMs = 200;                 // 0.2 sec
    static constexpr int kDiagnosticsPeriodMs = 500;              // 0.5 sec

    /// Timer used to detect startup timeout.
    QTimer* startupTimer_ = nullptr;

    /// Timer used for stream watchdog.
    QTimer* watchdogTimer_ = nullptr;

    /// Timer used for periodic diagnostics updates.
    QTimer* diagnosticsTimer_ = nullptr;

    /// Current streaming state.
    StreamingState state_ = StreamingState::Stopped;

    /// Current active device identifier.
    QByteArray deviceId_;

    //! Monotonic timestamp (ns) of the last valid frame, used for stream loss detection.
    int64_t lastValidFrameTsNs_{0};

    //! Timeout used to detect loss of video stream.

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    int testFrameCounter_{0};
#endif
};

} // namespace fluvel_app
