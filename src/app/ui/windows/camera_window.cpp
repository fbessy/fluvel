// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_window.hpp"
#include "autofit_behavior.hpp"
#include "fullscreen_behavior.hpp"
#include "icon_loader.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"

#include <QMediaDevices>
#include <QSettings>

#include <QCloseEvent>
#include <QImage>
#include <QSettings>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QVideoFrame>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QPermission>
#include <QtCore/qpermissions.h>
#endif

namespace fluvel_app
{

CameraWindow::CameraWindow(QWidget* parent)
    : QMainWindow(parent)

{
    setWindowIcon(QIcon(":/icons/app/fluvel.svg"));
    setWindowTitle(tr("Fluvel - Camera"));

    QSettings settings;

    if (settings.contains("ui_geometry/camera_window"))
    {
        restoreGeometry(settings.value("ui_geometry/camera_window").toByteArray());
    }
    else
    {
        resize(900, 600);
    }

    currentCameraId_ = settings.value("camera/device").toByteArray();

    cameraSelector_ = new QComboBox(this);
    cameraSelector_->setEnabled(false);

    startIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart, QStyle::SP_MediaPlay,
                              ":/icons/toolbar/media-playback-start-symbolic.svg");

    stopIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop, QStyle::SP_MediaStop,
                             ":/icons/toolbar/media-playback-stop-symbolic.svg");

    toggleStreamingButton_ = new QPushButton(tr("Start"));
    toggleStreamingButton_->setEnabled(false);
    toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
    toggleStreamingButton_->setIcon(startIcon_);

    rightPanelToggle_ = new RightPanelToggleButton;

    settingsButton_ = new QPushButton;
    settingsButton_->setToolTip(tr("Camera session settings"));
    settingsButton_->setFlat(true);
    settingsButton_->setFocusPolicy(Qt::NoFocus);

    settingsIcon_ = il::loadIcon("configure", ":/icons/toolbar/configure-symbolic.svg");

    settingsButton_->setIcon(settingsIcon_);

    settings_window_ =
        new CameraSettingsWindow(this, ApplicationSettings::instance().videoSettings());

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

    controlLayout->addSpacerItem(new QSpacerItem(12, 0, QSizePolicy::Fixed, QSizePolicy::Minimum));

    controlLayout->addWidget(settingsButton_);

    videoView_ =
        new ImageView(ApplicationSettings::instance().videoSettings().display,
                      ApplicationSettings::instance().videoSettings().compute.downscale, central);

    videoView_->setMaxDisplayFps(60.0);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    videoView_->setInteraction(interaction.release());

    // --- Display bar ---
    displayBar_ =
        new DisplaySettingsWidget(ApplicationSettings::instance().videoSettings().display, central);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(videoView_, 1);  // prend tout l'espace
    contentLayout->addWidget(displayBar_, 0); // largeur naturelle

    // Assemblage global
    vLayout->addWidget(controlBar);
    vLayout->addLayout(contentLayout);

    setCentralWidget(central);

    connect(rightPanelToggle_, &QPushButton::toggled, displayBar_,
            &DisplaySettingsWidget::setPanelVisible);

    connect(settingsButton_, &QPushButton::clicked, settings_window_, &CameraSettingsWindow::show);

    connect(toggleStreamingButton_, &QPushButton::clicked, this, &CameraWindow::onToggleStreaming);

    mediaDevices_ = new QMediaDevices(this);

    updateCameraList();

    connect(mediaDevices_, &QMediaDevices::videoInputsChanged, this,
            &CameraWindow::updateCameraList);

    controller_ = new CameraController(this);

    connect(controller_, &CameraController::frameSizeStr, this, &CameraWindow::onFrameSizeStr);

    connect(controller_, &CameraController::textStatsUpdated, videoView_, &ImageView::setText);

    videoView_->applyDownscaleConfig(
        ApplicationSettings::instance().videoSettings().compute.downscale);
    videoView_->applyDisplayConfig(ApplicationSettings::instance().videoSettings().display);

    connect(&ApplicationSettings::instance(), &ApplicationSettings::videoSettingsChanged, this,
            [this](const VideoSessionSettings& conf)
            {
                videoView_->applyDownscaleConfig(conf.compute.downscale);
            });

    connect(&ApplicationSettings::instance(), &ApplicationSettings::videoDisplaySettingsChanged,
            videoView_, &ImageView::applyDisplayConfig);

    connect(videoView_, &ImageView::frameDisplayed, controller_,
            &CameraController::onFrameDisplayed);

    connect(settings_window_, &CameraSettingsWindow::settingsAccepted,
            &ApplicationSettings::instance(), &ApplicationSettings::updateVideoSessionSettings);

    connect(displayBar_, &DisplaySettingsWidget::displayConfigChanged,
            &ApplicationSettings::instance(), &ApplicationSettings::setVideoDisplayConfig);

    bool preprocessing =
        ApplicationSettings::instance().videoSettings().compute.downscale.hasDownscale ||
        ApplicationSettings::instance().videoSettings().compute.hasTemporalFiltering;

    displayBar_->updatePipelineAvailability(preprocessing);

    connect(&ApplicationSettings::instance(), &ApplicationSettings::videoSettingsChanged, this,
            [this](const VideoSessionSettings& conf)
            {
                bool hasPreprocessing =
                    conf.compute.downscale.hasDownscale || conf.compute.hasTemporalFiltering;

                displayBar_->updatePipelineAvailability(hasPreprocessing);
            });
}

void CameraWindow::onFrameSizeStr(const QString& str)
{
    setWindowTitle(deviceWindowTitle_ + str);
}

void CameraWindow::updateCameraList()
{
    const auto cameras = QMediaDevices::videoInputs();

    cameraSelector_->clear();

    for (const auto& cam : cameras)
    {
        cameraSelector_->addItem(cam.description(), cam.id());
    }

    const bool hasCamera = !cameras.isEmpty();
    cameraSelector_->setEnabled(hasCamera);
    toggleStreamingButton_->setEnabled(hasCamera);

    if (!hasCamera)
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
    if (!controller_)
        return;

    if (controller_->isActive())
    {
        stopCameraAndUi();
    }
    else
    {
        const QByteArray selectedId = cameraSelector_->currentData().toByteArray();

        if (selectedId.isEmpty())
            return;

        currentCameraId_ = selectedId;

        QSettings settings;
        settings.setValue("camera/device", currentCameraId_);

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
    if (controller_ && controller_->isActive())
    {
        disconnect(frameConnection_);
        controller_->stop();
        videoView_->showPlaceholder(true);

        toggleStreamingButton_->setText(tr("Start"));
        toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
        toggleStreamingButton_->setIcon(startIcon_);

        setWindowTitle(tr("Fluvel - Camera"));
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

    settings.setValue("ui_geometry/camera_window", saveGeometry());

    emit cameraWindowClosed();
    QMainWindow::closeEvent(event);
}

#ifdef Q_OS_ANDROID
void CameraWindow::ensureCameraPermission()
{
    QCameraPermission permission;

    switch (qApp->checkPermission(permission))
    {
        case Qt::PermissionStatus::Undetermined:
            qApp->requestPermission(permission, this,
                                    [](const QPermission& p)
                                    {
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
    frameConnection_ = connect(controller_, &CameraController::imageAndContourUpdated, videoView_,
                               &ImageView::setImageAndContour);
}

} // namespace fluvel_app
