// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QMainWindow>

#include <QByteArray>
#include <QCamera>
#include <QIcon>
#include <QMetaObject>
#include <QSet>
#include <QString>

class QWidget;
class QComboBox;
class QPushButton;
class QLabel;

class QShowEvent;
class QCloseEvent;

namespace fluvel_app
{

struct StreamingInfo;
class CameraController;
class CameraSettingsWindow;
class RightPanelToggleButton;
class DisplaySettingsWidget;
class ImageViewerWidget;

/**
 * @brief Streaming status for a camera device.
 */
enum class DeviceStreamingStatus
{
    Idle,
    Streaming,
    Error
};

/**
 * @brief Stores a preferred camera format.
 *
 * A format is considered valid if it has a valid resolution
 * and pixel format.
 */
struct SavedFormat
{
    QSize resolution;
    float fps = 0.f;
    QVideoFrameFormat::PixelFormat pixelFormat = QVideoFrameFormat::Format_Invalid;

    bool isValid() const
    {
        return resolution.isValid() && pixelFormat != QVideoFrameFormat::Format_Invalid;
    }
};

/**
 * @brief RAII guard to temporarily disable UI updates.
 *
 * Sets the given flag to true on construction and restores it
 * to false on destruction.
 */
class ScopedUiUpdateGuard
{
public:
    explicit ScopedUiUpdateGuard(bool& flag)
        : flag_(flag)
    {
        flag_ = true;
    }

    ~ScopedUiUpdateGuard()
    {
        flag_ = false;
    }

private:
    bool& flag_;
};

/**
 * @brief Main window handling camera streaming and visualization.
 *
 * This window provides the user interface to:
 * - select a camera device and format
 * - start and stop video streaming
 * - display frames in an ImageViewerWidget
 * - configure processing and display settings
 *
 * It coordinates interactions between:
 * - CameraController (capture and streaming)
 * - ImageViewerWidget (display)
 * - settings widgets (camera and display)
 *
 * The class also manages device availability, preferred formats,
 * and streaming state.
 */
class CameraWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the camera window.
     *      * @param parent Optional parent widget.
     */
    explicit CameraWindow(QWidget* parent = nullptr);

    /**
     * @brief Returns whether at least one camera device is available.
     */
    bool isCameraAvailable() const;

signals:

    /**
     * @brief Emitted when camera availability changes.
     */
    void cameraAvailabilityChanged(bool available);

    /**
     * @brief Emitted when the window is shown.
     */
    void cameraWindowShown();

    /**
     * @brief Emitted when the window is closed.
     */
    void cameraWindowClosed();

protected:
    /**
     * @brief Handles window show events.
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Handles window close events.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    void onDownscaleChanged(const DownscaleParams& downscaleParams);
    void updateWindowTitle();

    void setupWindow();
    void restoreSettings();
    void createUi();
    static QIcon createActiveCameraIcon();
    static QIcon createEmptyIcon(int size);
    static QIcon createErrorCameraIcon();
    static QIcon createActiveFormatIcon();
    void setupView();
    void setupController();
    void setupLayout();
    void applyInitialSettings();
    void setupConnections();
    void bindApplicationSettingsToController();
    void bindApplicationSettingsToView();
    void bindUiToApplicationSettings();
    void connectFrameToView();

    void updateDeviceList(const QList<QCameraDevice>& devices);
    int computeBestDeviceIndex(const QByteArray& previousSelection, const QByteArray& newlyPlugged);
    void setDeviceControlsEnabled(bool enabled);
    void updateStreamingButton();
    void refreshUi();

    void onToggleStreaming();

    void onDeviceChanged(int index);
    void refreshFormatListFromSelection();
    void updateFormatList(const QList<QCameraFormat>& formats);
    static QString pixelFormatToShortString(QVideoFrameFormat::PixelFormat fmt);
    static QString formatToString(const QCameraFormat& fmt);
    QCameraFormat getSelectedFormat() const;

    int findBestFormatIndex(const QList<QCameraFormat>& formats) const;

    bool isYuv420(QVideoFrameFormat::PixelFormat fmt) const;
    bool isYuv(QVideoFrameFormat::PixelFormat fmt) const;
    bool is640x480(const QSize& s) const;
    bool is30fps(float fps) const;

    void loadPreferredFormats();
    void savePreferredFormats();

    void onStreamingStarted(const StreamingInfo& info);
    void onStreamingStopped();
    void onCameraError(const QByteArray& deviceId, QCamera::Error error,
                       const QString& errorString);
    void onStartupTimeout(const QByteArray& deviceId, double timeoutSec);
    void onStreamingLost(const QByteArray& deviceId, double frameAgeSec);

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    void startCamera();
    void stopCamera();

    static QByteArray loadSelectedCameraId();
    void saveSelectedCameraId();

    CameraSettingsWindow* cameraSettingsWindow_ = nullptr;

    QWidget* central_ = nullptr;

    QLabel* deviceLabel_ = nullptr;
    QComboBox* deviceSelector_ = nullptr;
    QLabel* formatLabel_ = nullptr;
    QComboBox* formatSelector_ = nullptr;
    QPushButton* toggleStreamingButton_ = nullptr;
    RightPanelToggleButton* rightPanelToggle_ = nullptr;
    QPushButton* settingsButton_ = nullptr;
    QIcon startIcon_;
    QIcon stopIcon_;
    QIcon settingsIcon_;

    DisplaySettingsWidget* displayBar_ = nullptr;

    QSet<QByteArray> knownDeviceIds_;
    QByteArray selectedDeviceId_;
    QByteArray streamingDeviceId_;

    ImageViewerWidget* imageViewer_ = nullptr;
    CameraController* cameraController_ = nullptr;
    QMetaObject::Connection frameToViewConnection_;

    QHash<QByteArray, DeviceStreamingStatus> deviceStreamingStatus_;

    QIcon deviceActiveIcon_;
    QIcon deviceIdleIcon_;
    QIcon deviceErrorIcon_;

    QIcon formatActiveIcon_;
    QIcon formatAvailableIcon_;

    bool switchingInProgress_{false};

    QHash<QByteArray, QCameraFormat> preferredFormats_;
    QCameraFormat activeFormat_;

    bool isUpdatingUi_{false};

    QString downscaleTitleStr_;
    QString deviceTitleStr_;
};

} // namespace fluvel_app
