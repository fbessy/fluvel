// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>

#include "application_settings.hpp"
#include "camera_controller.hpp"
#include "camera_settings_window.hpp"
#include "display_settings_widget.hpp"
#include "image_view.hpp"
#include "right_panel_toggle_button.hpp"
#include "video_active_contour_thread.hpp"

namespace ofeli_app
{

class CameraWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CameraWindow(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateCameraList();
    void onToggleStreaming();
    void onFrameSizeStr(const QString& str);

private:
    void connectFrameToView();
    void stopCameraAndUi();

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    CameraController* controller_;

    QByteArray currentCameraId_;

    QComboBox* cameraSelector_{nullptr};
    QPushButton* toggleStreamingButton_{nullptr};

    ImageView* videoView_{nullptr};

    QMediaDevices* mediaDevices_{nullptr};

    QString deviceWindowTitle_;

    QIcon startIcon_;
    QIcon stopIcon_;
    QIcon settingsIcon_;

    RightPanelToggleButton* rightPanelToggle_;
    QPushButton* settingsButton_;

    DisplaySettingsWidget* displayBar_;

    CameraSettingsWindow* settings_window_;

    QMetaObject::Connection frameConnection_;

signals:
    void cameraWindowShown();
    void cameraWindowClosed();
};

} // namespace ofeli_app
