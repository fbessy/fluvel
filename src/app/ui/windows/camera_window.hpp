// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

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
class QMediaDevices;

class QShowEvent;
class QCloseEvent;

namespace fluvel_app
{

class CameraSettingsWindow;
class RightPanelToggleButton;
class DisplaySettingsWidget;
class ImageView;
class CameraController;

enum class CameraState
{
    Normal,
    Active,
    Error
};

class CameraWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CameraWindow(QWidget* parent);

signals:
    void cameraWindowShown();
    void cameraWindowClosed();

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void setupWindow();
    void restoreSettings();
    void createUi();
    static QIcon createActiveCameraIcon();
    static QIcon createEmptyCameraIcon();
    static QIcon createErrorCameraIcon();
    void setupView();
    void setupController();
    void setupLayout();
    void applyInitialSettings();
    void setupConnections();
    void bindApplicationSettingsToController();
    void bindApplicationSettingsToView();
    void bindUiToApplicationSettings();
    void connectFrameToView();

    void updateCameraList();
    int computeBestCameraIndex(const QByteArray& previousSelection, const QByteArray& newlyPlugged);
    void setCameraControlsEnabled(bool enabled);
    void updateStreamingButton();
    void refreshUi();

    void onToggleStreaming();

    void onCameraStarted(const QByteArray& deviceId);
    void onCameraStopped();
    void onCameraError(const QByteArray& deviceId, QCamera::Error error,
                       const QString& errorString);
    void onStreamingLost(const QByteArray& deviceId, qint64 timeoutNs);

    void onFrameSizeStr(const QString& str);

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    void startCamera();
    void stopCamera();

    static QByteArray loadSelectedCameraId();
    void saveSelectedCameraId();

    CameraSettingsWindow* cameraSettingsWindow_ = nullptr;

    QWidget* central_ = new QWidget(this);

    QComboBox* cameraSelector_ = nullptr;
    QPushButton* toggleStreamingButton_ = nullptr;
    RightPanelToggleButton* rightPanelToggle_ = nullptr;
    QPushButton* settingsButton_ = nullptr;
    QIcon startIcon_;
    QIcon stopIcon_;
    QIcon settingsIcon_;

    DisplaySettingsWidget* displayBar_ = nullptr;

    QMediaDevices* mediaDevices_ = nullptr;
    QByteArray activeCameraId_;
    QSet<QByteArray> knownCameraIds_;

    ImageView* videoView_ = nullptr;
    CameraController* cameraController_ = nullptr;
    QMetaObject::Connection frameToViewConnection_;

    QString deviceWindowTitle_;

    QIcon activeCameraIcon_;
    QIcon emptyCameraIcon_;
    QIcon errorCameraIcon_;

    bool switchingInProgress_ = false;

    QHash<QByteArray, CameraState> cameraStates_;
};

} // namespace fluvel_app
