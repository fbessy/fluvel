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
#include <QTimer>
#include <QSettings>

namespace ofeli_app
{

CameraWindow::CameraWindow(QWidget* parent)
    : QMainWindow(parent),
    cameraSelector(nullptr),
    toggleStreamingButton(nullptr),
    stacked(nullptr),
    blackLabel(nullptr),
    videoView(nullptr),
    mediaDevices(nullptr),
    camera(nullptr),
    captureSession(nullptr),
    videoSink(nullptr),
    ac_thread(nullptr),
    nextFrameId(0),
    statsTimer(nullptr),
    lastFrameReceiveTs(0)
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

    videoView = new ImageView(this);
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

    statsTimer = new QTimer(this);
    statsTimer->setInterval(500);
}

CameraWindow::~CameraWindow()
{
    stopCamera();
}

void CameraWindow::startCamera(const QCameraDevice& device)
{
    if ( device.isNull() )
        return;


    stopCamera();

    camera = new QCamera(device, this);
    videoSink = new QVideoSink(this);
    captureSession = new QMediaCaptureSession(this);

    captureSession->setCamera(camera);
    captureSession->setVideoSink(videoSink);
    camera->start();

    ac_thread = new VideoActiveContourThread(this);

    connect(videoSink, &QVideoSink::videoFrameChanged,
            this, [this](const QVideoFrame& frame)
            {
                const qint64 recvTs = FrameClock::nowNs();

                frameStats.frameReceived(recvTs);   // ✅ input
                ac_thread->submitFrame(frame);      // envoi thread
            });

    connect(ac_thread, &VideoActiveContourThread::frameProcessed,
            this, [this]()
            {
                frameStats.frameProcessed();        // ✅ processing
            },
            Qt::QueuedConnection);

    connect(ac_thread, &VideoActiveContourThread::frameResultReady,
            this, [this](const QImage& img, qint64 recvTs)
            {
                videoView->setImage(img);

                const qint64 displayTs = FrameClock::nowNs();
                frameStats.frameDisplayed(recvTs, displayTs); // ✅ display + latence
            },
            Qt::QueuedConnection);

    ac_thread->start();

    statsTimer->start();

    connect(statsTimer, &QTimer::timeout,
            this, &CameraWindow::updateStatsUi);

    connect(this,
            &CameraWindow::cameraStatsUpdated,
            cameraOverlay,
            &CameraOverlayWidget::setStats);

    deviceWindowTitle = "Ofeli - " + device.description() + " - ";
    connect(ac_thread, &VideoActiveContourThread::frameSizeStr,
            this, &CameraWindow::onFrameSizeStr);

    frameStats.reset();
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

    if (!hasCamera)
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
    // --- Si déjà actif → stop ---
    if (camera != nullptr && camera->isActive())
    {
        stopCameraAndUi();
        return;
    }

    // --- Récupérer l'ID sélectionné ---
    const QByteArray selectedId =
        cameraSelector->currentData().toByteArray();

    if (selectedId.isEmpty())
        return;

    // --- Sauvegarde persistante ---
    currentCameraId = selectedId;

    QSettings settings;
    settings.setValue("Camera/last_id", currentCameraId);

    // --- Chercher le device correspondant ---
    const auto cameras = QMediaDevices::videoInputs();

    for (const auto& cam : cameras)
    {
        if (cam.id() == selectedId)
        {
            stacked->setCurrentIndex(1);
            startCamera(cam);

            toggleStreamingButton->setText(tr("Stop"));
            toggleStreamingButton->setToolTip(tr("Stop camera streaming."));
            toggleStreamingButton->setIcon(stopIcon);
            return;
        }
    }

    // --- Sécurité : si ID non trouvé ---
    // (ex: changement ultra-rapide device)
    stopCameraAndUi();
    currentCameraId.clear();
}

void CameraWindow::stopCamera()
{
    if ( statsTimer != nullptr ) {
        statsTimer->stop();
    }

    if( ac_thread != nullptr )
    {
        ac_thread->disconnect(this);

        ac_thread->stop();
        ac_thread->wait();

        ac_thread->deleteLater();
        ac_thread = nullptr;
    }

    if( captureSession != nullptr ) {
        captureSession->deleteLater();
        captureSession = nullptr;
    }

    if ( videoSink != nullptr )
    {
        videoSink->disconnect(this);

        videoSink->deleteLater();
        videoSink = nullptr;
    }

    if ( camera != nullptr )
    {
        camera->stop();
        camera->deleteLater();
        camera = nullptr;
    }
}

void CameraWindow::stopCameraAndUi()
{
    if ( camera != nullptr && camera->isActive() )
    {
        stacked->setCurrentIndex(0);
        stopCamera();

        toggleStreamingButton->setText( tr("Start") );
        toggleStreamingButton->setToolTip(tr("Start camera streaming."));
        toggleStreamingButton->setIcon( startIcon );

        setWindowTitle( tr("Ofeli - Camera") );
    }
}

void CameraWindow::updateStatsUi()
{
    auto snap = frameStats.snapshot();

    CameraStatsUi stats {
        snap.inputFps,
        snap.processingFps,
        snap.displayFps,
        snap.dropRate,
        snap.avgLatencyMs,
        snap.maxLatencyMs
    };

    emit cameraStatsUpdated(stats);
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

}
