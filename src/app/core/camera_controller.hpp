// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"
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

enum class StreamingState
{
    Stopped,
    Starting,
    Streaming
};

class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(const VideoSessionSettings& session, QObject* parent = nullptr);
    ~CameraController() override;

    void start(const QByteArray& deviceId, const QCameraFormat& format);
    void stop();
    bool isStreaming() const;

    QList<QCameraDevice> videoInputs() const;

    void onVideoSettingsChanged(const VideoSessionSettings& session);
    void onVideoDisplaySettingsChanged(const DisplayConfig& display);

    void onFrameDisplayed(const FrameTimestamps& ts);

signals:
    void videoInputsChanged(const QList<QCameraDevice>& inputs);
    void streamingStarted(const QByteArray& deviceId);
    void streamingStopped();
    void cameraError(const QByteArray& deviceId, QCamera::Error error, const QString& errorString);
    void startupTimeout(const QByteArray& deviceId, double timeoutSec);
    void streamingLost(const QByteArray& deviceId, double frameAgeSec);

    void frameSizeStr(const QString& str);
    void textStatsUpdated(const QString& textStats);
    void imageAndContourUpdated(const UiFrame& uiFrame);

private:
    QCameraFormat chooseBestFormat(const QCameraDevice& dev);

    void onVideoInputsChanged();
    void handleActiveDeviceUnplug(const QList<QCameraDevice>& inputs);

    void onCameraError(QCamera::Error error, const QString& errorString);
    void onCapturedFrame(const QVideoFrame& frame);

    void onFrameProcessed(quint64 contourSize);
    void onDisplayFrameReady(const DisplayFrame& displayFrame);
    void onStartupTimeout();
    void checkWatchdog();
    void updateDiagnostics();

    bool useOptimizedFormat_{true};
    QCamera* camera_ = nullptr;
    QMediaCaptureSession* captureSession_ = nullptr;
    QVideoSink* videoSink_ = nullptr;

    VideoActiveContourThread activeContourThread_;

    FrameStatsView frameStats_;

    // --- Timing configuration ---

    static constexpr int kStartupTimeoutMs = 7'000;               // 7 sec
    static constexpr qint64 kStreamLossTimeoutNs = 2'000'000'000; // 2 sec
    static constexpr int kWatchdogPeriodMs = 200;                 // 0.2 sec
    static constexpr int kDiagnosticsPeriodMs = 500;              // 0.5 sec

    QTimer* startupTimer_ = nullptr;
    QTimer* watchdogTimer_ = nullptr;
    QTimer* diagnosticsTimer_ = nullptr;

    DisplayConfig displayConfig_;
    StreamingState state_ = StreamingState::Stopped;
    QByteArray deviceId_;

    //! Monotonic timestamp (ns) of the last valid frame, used for stream loss detection.
    qint64 lastValidFrameTsNs_{0};

    //! Timeout used to detect loss of video stream.

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    int testFrameCounter_{0};
#endif
};

} // namespace fluvel_app
