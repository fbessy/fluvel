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
    : QMainWindow(parent)
    
{
    setWindowIcon( QIcon(":/icons/app/Ofeli.svg") );
    setWindowTitle( tr("Ofeli - Camera") );

    QSettings settings;
    const auto geo = settings.value("Camera/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    currentCameraId_ = settings.value("Camera/last_id").toByteArray();

    cameraSelector_ = new QComboBox(this);
    cameraSelector_->setEnabled(false);

    startIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                             QStyle::SP_MediaPlay,
                             ":/icons/toolbar/media-playback-start-symbolic.svg");

    stopIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                            QStyle::SP_MediaStop,
                            ":/icons/toolbar/media-playback-stop-symbolic.svg");

    toggleStreamingButton_ = new QPushButton( tr("Start") );
    toggleStreamingButton_->setEnabled(false);
    toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
    toggleStreamingButton_->setIcon( startIcon_ );

    rightPanelToggle_ = new RightPanelToggleButton;

    settingsButton_ = new QPushButton;
    settingsButton_->setToolTip(tr("Camera session settings"));
    settingsButton_->setFlat(true);
    settingsButton_->setFocusPolicy(Qt::NoFocus);

    settingsIcon_ = il::loadIcon("configure",
                                ":/icons/toolbar/configure-symbolic.svg");

    settingsButton_->setIcon( settingsIcon_ );

    settings_window_ = new CameraSettingsWindow(this);

    QWidget* central = new QWidget(this);

    // Layout principal vertical
    QVBoxLayout* vLayout = new QVBoxLayout(central);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    QWidget* controlBar = new QWidget(central);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(6);
    controlLayout->addWidget(cameraSelector_);
    controlLayout->addWidget(toggleStreamingButton_);
    controlLayout->addStretch();

    controlLayout->addWidget(rightPanelToggle_);

    controlLayout->addSpacerItem(
        new QSpacerItem(12, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)
        );

    controlLayout->addWidget(settingsButton_);

    videoView_ = new ImageView(AppSettings::instance().camConfig.display,
                               AppSettings::instance().camConfig.compute.downscale,
                               central);

    videoView_->setMaxDisplayFps(60.0);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    videoView_->setInteraction(interaction.release());

    // --- Display bar ---
    displayBar_ = new DisplaySettingsWidget(central,
                                           Session::Camera);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(videoView_,  1);  // prend tout l'espace
    contentLayout->addWidget(displayBar_, 0); // largeur naturelle

    // Assemblage global
    vLayout->addWidget(controlBar);
    vLayout->addLayout(contentLayout);

    setCentralWidget(central);

    connect(rightPanelToggle_, &QPushButton::toggled,
            displayBar_, &DisplaySettingsWidget::setPanelVisible);

    connect(settingsButton_,     &QPushButton::clicked,
            settings_window_,    &CameraSettingsWindow::show);

    connect(toggleStreamingButton_,  &QPushButton::clicked,
            this,                   &CameraWindow::onToggleStreaming);

    mediaDevices_ = new QMediaDevices(this);

    updateCameraList();

    connect(mediaDevices_,
            &QMediaDevices::videoInputsChanged,
            this,
            &CameraWindow::updateCameraList);

    controller_ = new CameraController(this);

    connect(controller_,
            &CameraController::frameSizeStr,
            this,
            &CameraWindow::onFrameSizeStr);

    connect(controller_,
            &CameraController::textStatsUpdated,
            videoView_,
            &ImageView::setText);

    videoView_->applyDownscaleConfig( AppSettings::instance().camConfig.compute.downscale );
    videoView_->applyDisplayConfig( AppSettings::instance().camConfig.display );

    connect(&AppSettings::instance(), &ApplicationSettings::videoSettingsChanged,
            this, [this](const VideoSessionSettings& conf) {
                videoView_->applyDownscaleConfig( conf.compute.downscale );
            });

    connect(&AppSettings::instance(),
            &ApplicationSettings::videoDisplaySettingsChanged,
            videoView_,
            &ImageView::applyDisplayConfig);

    connect(videoView_,  &ImageView::frameDisplayed,
            controller_, &CameraController::onFrameDisplayed);
}

void CameraWindow::onFrameSizeStr(const QString& str)
{
    setWindowTitle( deviceWindowTitle_ + str );
}

void CameraWindow::updateCameraList()
{
    const auto cameras = QMediaDevices::videoInputs();

    cameraSelector_->clear();

    for (const auto& cam : cameras)
    {
        cameraSelector_->addItem(
            cam.description(),
            cam.id()
            );
    }

    const bool hasCamera = !cameras.isEmpty();
    cameraSelector_->setEnabled(hasCamera);
    toggleStreamingButton_->setEnabled(hasCamera);

    if ( !hasCamera )
    {
        // plus aucune caméra disponible
        stopCameraAndUi();
        currentCameraId_.clear();
        return;
    }

    // 🔎 Vérifier si la caméra active existe encore
    if (!currentCameraId_.isEmpty())
    {
        int index = cameraSelector_->findData(currentCameraId_);

        if (index >= 0)
        {
            // caméra toujours présente → restaurer sélection
            cameraSelector_->setCurrentIndex(index);
        }
        else
        {
            // caméra débranchée à chaud
            stopCameraAndUi();
            currentCameraId_.clear();

            // fallback propre : sélectionner la première dispo
            cameraSelector_->setCurrentIndex(0);
        }
    }
    else
    {
        // aucun historique → sélectionner première caméra
        cameraSelector_->setCurrentIndex(0);
    }
}

void CameraWindow::onToggleStreaming()
{
    if ( !controller_ )
        return;

    if ( controller_->isActive() )
    {
        stopCameraAndUi();
    }
    else
    {
        const QByteArray selectedId =
            cameraSelector_->currentData().toByteArray();

        if (selectedId.isEmpty())
            return;

        currentCameraId_ = selectedId;

        QSettings settings;
        settings.setValue("Camera/last_id", currentCameraId_);

        videoView_->showPlaceholder(false);
        controller_->start(selectedId);
        connectFrameToView();

        toggleStreamingButton_->setText(tr("Stop"));
        toggleStreamingButton_->setToolTip(tr("Stop camera streaming."));
        toggleStreamingButton_->setIcon(stopIcon_);
    }
}

void CameraWindow::stopCameraAndUi()
{
    if ( controller_ && controller_->isActive() )
    {
        disconnect(frameConnection_);
        controller_->stop();
        videoView_->showPlaceholder(true);

        toggleStreamingButton_->setText( tr("Start") );
        toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
        toggleStreamingButton_->setIcon( startIcon_ );

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

void CameraWindow::connectFrameToView()
{
    frameConnection_ = connect(controller_,
                               &CameraController::imageAndContourUpdated,
                               videoView_,
                               &ImageView::setImageAndContour);
}

}
