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

#include <QSettings>
#include <QMediaDevices>

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QVideoFrame>
#include <QImage>
#include <QCloseEvent>
#include <QTimer>

namespace ofeli_app
{

CameraWindow::CameraWindow(QWidget* parent)
    : QDialog(parent),
    cameraSelector(nullptr),
    labels(nullptr),
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
    setWindowTitle( tr("Camera") );

    QSettings settings;
    const auto geo = settings.value("Camera/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    blackLabel = new QLabel(this);
    blackLabel->setAlignment(Qt::AlignCenter);
    QImage blackImage(640, 480, QImage::Format_RGB32);
    blackImage.fill(Qt::black);
    blackLabel->setPixmap(QPixmap::fromImage(blackImage));

    videoView = new ImageView(this);
    videoView->setMaxDisplayFps(60.0);

    cameraOverlay = new CameraOverlayWidget(this);



    QString info = QString("=================================");

    QWidget* viewContainer = new QWidget(this);
    QStackedLayout* overlayStack = new QStackedLayout(viewContainer);
    overlayStack->setStackingMode(QStackedLayout::StackAll);

    overlayStack->addWidget(videoView);
    overlayStack->addWidget(cameraOverlay);
    cameraOverlay->raise();

    labels = new QStackedWidget(this);
    labels->addWidget(blackLabel);
    labels->addWidget(viewContainer);


    // État initial
    labels->setCurrentIndex(0);

    cameraSelector = new QComboBox(this);
    mediaDevices = new QMediaDevices(this);

    connect(cameraSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraWindow::onCameraSelected);

    connect(mediaDevices,
            &QMediaDevices::videoInputsChanged,
            this,
            &CameraWindow::updateCameraList);

    updateCameraList();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(cameraSelector);
    layout->addWidget(labels);
    setLayout(layout);

    statsTimer = new QTimer(this);
    statsTimer->setInterval(500);
}

CameraWindow::~CameraWindow()
{
    stopCamera();
}

void CameraWindow::startCamera(const QCameraDevice& device)
{
    if ( !device.isNull() )
    {
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
                    videoView->displayImage(img);

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

        frameStats.reset();

        labels->setCurrentIndex(1);
    }
}

void CameraWindow::updateCameraList()
{
    cameraSelector->blockSignals(true);
    cameraSelector->clear();

    cameraSelector->addItem(tr("Éteint"));

    const auto cameras = QMediaDevices::videoInputs();
    for (const auto &cam : cameras) {
        cameraSelector->addItem(cam.description());
    }

    cameraSelector->setCurrentIndex(0);
    cameraSelector->setEnabled(!cameras.isEmpty());

    cameraSelector->blockSignals(false);
}

void CameraWindow::onCameraSelected(int index)
{
    if (index == 0) {
        stopCamera();
        return;
    }

    const auto cameras = QMediaDevices::videoInputs();
    const int camIndex = index - 1;

    if (camIndex >= 0 && camIndex < cameras.size()) {
        startCamera(cameras[camIndex]);
    }
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

    labels->setCurrentIndex(0);
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

void CameraWindow::closeEvent(QCloseEvent* event)
{
    stopCamera();

    QSettings settings;

    settings.setValue( "Camera/Window/geometry", saveGeometry() );

    QDialog::closeEvent(event);
}

}
