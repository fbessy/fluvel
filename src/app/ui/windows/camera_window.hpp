// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "camera_controller.hpp"
#include "camera_settings_window.hpp"
#include "display_settings_widget.hpp"
#include "image_view.hpp"
#include "right_panel_toggle_button.hpp"

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>

namespace fluvel_app
{

class CameraWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CameraWindow(QWidget* parent = nullptr);

signals:
    void cameraWindowShown();
    void cameraWindowClosed();

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void updateCameraList();
    void onToggleStreaming();
    void onFrameSizeStr(const QString& str);

    void bindApplicationSettings();
    void connectFrameToView();
    void stopCameraAndUi();

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    CameraController* cameraController_ = nullptr;

    QByteArray currentCameraId_;

    QComboBox* cameraSelector_ = nullptr;
    QPushButton* toggleStreamingButton_ = nullptr;

    ImageView* videoView_ = nullptr;

    QMediaDevices* mediaDevices_ = nullptr;

    QString deviceWindowTitle_;

    QIcon startIcon_;
    QIcon stopIcon_;
    QIcon settingsIcon_;

    RightPanelToggleButton* rightPanelToggle_ = nullptr;
    QPushButton* settingsButton_ = nullptr;

    DisplaySettingsWidget* displayBar_ = nullptr;

    CameraSettingsWindow* cameraSettingsWindow_ = nullptr;

    QMetaObject::Connection frameConnection_;
};

} // namespace fluvel_app
