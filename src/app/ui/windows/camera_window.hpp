/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef CAMERA_WINDOW_HPP
#define CAMERA_WINDOW_HPP

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

#include "camera_controller.hpp"
#include "video_active_contour_thread.hpp"
#include "application_settings.hpp"
#include "image_view.hpp"
#include "display_settings_widget.hpp"
#include "camera_settings_window.hpp"
#include "camera_overlay_widget.hpp"

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
    void onFrameSizeStr(QString str);

private:

    void stopCameraAndUi();

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    CameraController* controller;

    QByteArray currentCameraId;

    QComboBox* cameraSelector;
    QPushButton* toggleStreamingButton;

    QStackedWidget* stacked;
    ImageView* videoView;
    CameraOverlayWidget* cameraOverlay;

    QMediaDevices*         mediaDevices;

    QString deviceWindowTitle;

    QIcon startIcon;
    QIcon stopIcon;
    QIcon settingsIcon;

    QPushButton* rightPanelToggle;
    QPushButton* settingsButton;

    DisplaySettingsWidget* displayBar;

    CameraSettingsWindow* settings_window;

signals:
    void cameraWindowShown();
    void cameraWindowClosed();
};

}

#endif // CAMERA_WINDOW_HPP
