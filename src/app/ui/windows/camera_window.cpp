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

#include "camera_window.hpp"
#include "frame_clock.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "fullscreen_behavior.hpp"
#include "autofit_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "icon_loader.hpp"

#include <QSettings>
#include <QMediaDevices>

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QVideoFrame>
#include <QImage>
#include <QCloseEvent>
#include <QSettings>

#ifdef Q_OS_ANDROID
    #include <QCoreApplication>
    #include <QPermission>
    #include <QtCore/qpermissions.h>
#endif

namespace ofeli_app
{

CameraWindow::CameraWindow(QWidget* parent)
    : QMainWindow(parent),
    cameraSelector(nullptr),
    toggleStreamingButton(nullptr),
    stacked(nullptr),
    blackLabel(nullptr),
    videoView(nullptr),
    mediaDevices(nullptr)
{
    setWindowIcon( QIcon(":/icons/app/Ofeli.svg") );
    setWindowTitle( tr("Ofeli - Camera") );

    QSettings settings;
    const auto geo = settings.value("Camera/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    currentCameraId = settings.value("Camera/last_id").toByteArray();

    blackLabel = new QLabel(this);
    blackLabel->setAlignment(Qt::AlignCenter);
    QImage blackImage(640, 480, QImage::Format_RGB32);
    blackImage.fill(Qt::black);
    blackLabel->setPixmap(QPixmap::fromImage(blackImage));

    videoView = new ImageView(AppSettings::instance().camConfig.display,
                              AppSettings::instance().camConfig.compute.downscale,
                              this);

    videoView->setMaxDisplayFps(60.0);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    videoView->setInteraction(interaction.release());

    cameraOverlay = new CameraOverlayWidget(this);



    QString info = QString("=================================");

    QWidget* viewContainer = new QWidget(this);
    QStackedLayout* overlayStack = new QStackedLayout(viewContainer);
    overlayStack->setStackingMode(QStackedLayout::StackAll);

    overlayStack->addWidget(videoView);
    overlayStack->addWidget(cameraOverlay);
    cameraOverlay->raise();

    stacked = new QStackedWidget(this);
    stacked->addWidget(blackLabel);
    stacked->addWidget(viewContainer);

    cameraSelector = new QComboBox(this);
    cameraSelector->setEnabled(false);

    startIcon = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                             QStyle::SP_MediaPlay,
                             ":/icons/toolbar/media-playback-start-symbolic.svg");

    stopIcon = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                            QStyle::SP_MediaStop,
                            ":/icons/toolbar/media-playback-stop-symbolic.svg");

    toggleStreamingButton = new QPushButton( tr("Start") );
    toggleStreamingButton->setEnabled(false);
    toggleStreamingButton->setToolTip(tr("Start camera streaming."));
    toggleStreamingButton->setIcon( startIcon );

    rightPanelToggle = new QPushButton;
    rightPanelToggle->setCheckable(true);
    rightPanelToggle->setChecked(true);
    rightPanelToggle->setFocusPolicy(Qt::NoFocus);
    rightPanelToggle->setToolTip(tr("Right panel is visible."));

    rightPanelToggle->setIcon(QIcon(":/icons/toolbar/right_panel_on.svg"));

    settingsButton = new QPushButton;
    settingsButton->setToolTip(tr("Segmentation settings"));
    settingsButton->setFlat(true);
    settingsButton->setFocusPolicy(Qt::NoFocus);

    settingsIcon = il::loadIcon("configure",
                                ":/icons/toolbar/configure-symbolic.svg");

    settingsButton->setIcon( settingsIcon );

    settings_window = new CameraSettingsWindow(this);

    QWidget* central = new QWidget(this);

    QWidget* controlBar = new QWidget(central);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(6);
    controlLayout->addWidget(cameraSelector);
    controlLayout->addWidget(toggleStreamingButton);
    controlLayout->addStretch();

    controlLayout->addWidget(rightPanelToggle);

    controlLayout->addSpacerItem(
        new QSpacerItem(12, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)
        );

    controlLayout->addWidget(settingsButton);

    // --- Display bar ---
    displayBar = new DisplaySettingsWidget(central,
                                           Session::Camera);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(stacked, 1);   // zone principale extensible
    contentLayout->addWidget(displayBar);   // panneau droit

    // --- Layout vertical global ---
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(controlBar);
    mainLayout->addLayout(contentLayout);

    setCentralWidget(central);

    connect(rightPanelToggle, &QPushButton::toggled,
            this, [this](bool checked)
            {
                if ( checked )
                {
                    rightPanelToggle->setIcon(QIcon(":/icons/toolbar/right_panel_on.svg"));
                    rightPanelToggle->setToolTip(tr("Right panel is visible."));
                }
                else
                {
                    rightPanelToggle->setIcon(QIcon(":/icons/toolbar/right_panel_off.svg"));
                    rightPanelToggle->setToolTip(tr("Right panel is hidden."));
                }

                displayBar->setVisible(checked);
            });

    connect(settingsButton,     &QPushButton::clicked,
            settings_window,    &CameraSettingsWindow::show);

    connect(toggleStreamingButton,  &QPushButton::clicked,
            this,                   &CameraWindow::onToggleStreaming);

    mediaDevices = new QMediaDevices(this);

    updateCameraList();

    connect(mediaDevices,
            &QMediaDevices::videoInputsChanged,
            this,
            &CameraWindow::updateCameraList);

    controller = new CameraController(this);

    connect(controller,
            &CameraController::frameSizeStr,
            this,
            &CameraWindow::onFrameSizeStr);

    connect(controller,
            &CameraController::statsUpdated,
            cameraOverlay,
            &CameraOverlayWidget::setStats);

    videoView->applyDownscaleConfig( AppSettings::instance().camConfig.compute.downscale );
    videoView->applyDisplayConfig( AppSettings::instance().camConfig.display );

    connect(&AppSettings::instance(), &ApplicationSettings::videoSettingsChanged,
            this, [this](const VideoSessionSettings& conf) {
                videoView->applyDownscaleConfig( conf.compute.downscale );
            });

    connect(&AppSettings::instance(),
            &ApplicationSettings::videoDisplaySettingsChanged,
            videoView,
            &ImageView::applyDisplayConfig);

    connect(controller,
            &CameraController::imageAndContourUpdated,
            videoView,
            &ImageView::setImageAndContour);

    connect(videoView,  &ImageView::frameDisplayed,
            controller, &CameraController::onFrameDisplayed);
}

void CameraWindow::onFrameSizeStr(QString str)
{
    setWindowTitle( deviceWindowTitle + str );
}

void CameraWindow::updateCameraList()
{
    const auto cameras = QMediaDevices::videoInputs();

    cameraSelector->clear();

    for (const auto& cam : cameras)
    {
        cameraSelector->addItem(
            cam.description(),
            cam.id()
            );
    }

    const bool hasCamera = !cameras.isEmpty();
    cameraSelector->setEnabled(hasCamera);
    toggleStreamingButton->setEnabled(hasCamera);

    if ( !hasCamera )
    {
        // plus aucune caméra disponible
        stopCameraAndUi();
        currentCameraId.clear();
        return;
    }

    // 🔎 Vérifier si la caméra active existe encore
    if (!currentCameraId.isEmpty())
    {
        int index = cameraSelector->findData(currentCameraId);

        if (index >= 0)
        {
            // caméra toujours présente → restaurer sélection
            cameraSelector->setCurrentIndex(index);
        }
        else
        {
            // caméra débranchée à chaud
            stopCameraAndUi();
            currentCameraId.clear();

            // fallback propre : sélectionner la première dispo
            cameraSelector->setCurrentIndex(0);
        }
    }
    else
    {
        // aucun historique → sélectionner première caméra
        cameraSelector->setCurrentIndex(0);
    }
}

void CameraWindow::onToggleStreaming()
{
    if ( !controller )
        return;

    if ( controller->isActive() )
    {
        stopCameraAndUi();
    }
    else
    {
        const QByteArray selectedId =
            cameraSelector->currentData().toByteArray();

        if (selectedId.isEmpty())
            return;

        currentCameraId = selectedId;

        QSettings settings;
        settings.setValue("Camera/last_id", currentCameraId);

        stacked->setCurrentIndex(1);
        controller->start(selectedId);

        toggleStreamingButton->setText(tr("Stop"));
        toggleStreamingButton->setToolTip(tr("Stop camera streaming."));
        toggleStreamingButton->setIcon(stopIcon);
    }
}

void CameraWindow::stopCameraAndUi()
{
    if ( controller && controller->isActive() )
    {
        stacked->setCurrentIndex(0);
        controller->stop();

        toggleStreamingButton->setText( tr("Start") );
        toggleStreamingButton->setToolTip(tr("Start camera streaming."));
        toggleStreamingButton->setIcon( startIcon );

        setWindowTitle( tr("Ofeli - Camera") );
    }
}

void CameraWindow::showEvent(QShowEvent* event)
{
    emit cameraWindowShown();
    QMainWindow::showEvent(event);
}

void CameraWindow::closeEvent(QCloseEvent* event)
{
    stopCameraAndUi();

    QSettings settings;

    settings.setValue( "Camera/Window/geometry", saveGeometry() );

    emit cameraWindowClosed();
    QMainWindow::closeEvent(event);
}

#ifdef Q_OS_ANDROID
void CameraWindow::ensureCameraPermission()
{
    QCameraPermission permission;

    switch (qApp->checkPermission(permission)) {

    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(permission, this,
                                [](const QPermission &p) {
                                    if (p.status() != Qt::PermissionStatus::Granted)
                                        qWarning() << "Camera permission denied";
                                });
        break;

    case Qt::PermissionStatus::Denied:
        qWarning() << "Camera permission denied";
        break;

    case Qt::PermissionStatus::Granted:
        break;
    }
}
#endif

}
