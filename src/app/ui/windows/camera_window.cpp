// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_window.hpp"
#include "application_settings.hpp"
#include "autofit_behavior.hpp"
#include "camera_controller.hpp"
#include "camera_settings_window.hpp"
#include "display_settings_widget.hpp"
#include "fullscreen_behavior.hpp"
#include "icon_loader.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "right_panel_toggle_button.hpp"

#include <QCameraDevice>
#include <QComboBox>
#include <QMediaDevices>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

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
    setupWindow();
    restoreSettings();

    createUi();
    setupView();
    setupController();
    setupLayout();

    applyInitialSettings();
    setupConnections();
}

void CameraWindow::setupWindow()
{
    setWindowIcon(QIcon(":/icons/app/fluvel.svg"));
    setWindowTitle(tr("Fluvel - Camera"));
}

void CameraWindow::restoreSettings()
{
    QSettings settings;

    if (settings.contains("ui_geometry/camera_window"))
        restoreGeometry(settings.value("ui_geometry/camera_window").toByteArray());
    else
        resize(900, 600);
}

void CameraWindow::createUi()
{
    const auto& config = ApplicationSettings::instance().videoSettings();

    central_ = new QWidget(this);

    cameraSelector_ = new QComboBox(this);

    // Adjust width to contents (needed when items have icons)
    cameraSelector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cameraSelector_->setIconSize(QSize(13, 13));
    cameraSelector_->setEnabled(false);

    activeCameraIcon_ = createActiveCameraIcon();
    emptyCameraIcon_ = createEmptyCameraIcon();
    errorCameraIcon_ = createErrorCameraIcon();

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

    // --- Display bar ---
    displayBar_ = new DisplaySettingsWidget(config.display, central_);

    cameraSettingsWindow_ = new CameraSettingsWindow(this, config);
}

QIcon CameraWindow::createActiveCameraIcon()
{
    const int pixSize = 13;
    const int ellipseSize = 11;

    QPixmap pix(pixSize, pixSize);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    p.setBrush(QColor(0, 200, 0));
    p.setPen(QPen(QColor(40, 40, 40), 1));

    p.drawEllipse(1, 1, ellipseSize, ellipseSize);

    return QIcon(pix);
}

QIcon CameraWindow::createEmptyCameraIcon()
{
    QPixmap pix(13, 13);
    pix.fill(Qt::transparent);
    return QIcon(pix);
}

QIcon CameraWindow::createErrorCameraIcon()
{
    const int pixSize = 13;
    const int ellipseSize = 11;

    QPixmap pix(pixSize, pixSize);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    p.setBrush(QColor(255, 165, 0)); // orange
    p.setPen(QPen(QColor(40, 40, 40), 1));

    p.drawEllipse(1, 1, ellipseSize, ellipseSize);

    return QIcon(pix);
}

void CameraWindow::setupView()
{
    const auto& app = ApplicationSettings::instance();

    videoView_ =
        new ImageView(app.videoSettings().display, app.videoSettings().compute.downscale, central_);

    videoView_->setMaxDisplayFps(60.0);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    videoView_->setInteraction(interaction.release());
}

void CameraWindow::setupController()
{
    const auto& config = ApplicationSettings::instance().videoSettings();
    cameraController_ = new CameraController(config, this);

    mediaDevices_ = new QMediaDevices(this);
}

void CameraWindow::setupLayout()
{
    // Layout principal vertical
    QVBoxLayout* vLayout = new QVBoxLayout(central_);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    QWidget* controlBar = new QWidget(central_);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(6);
    controlLayout->addWidget(cameraSelector_);
    controlLayout->addWidget(toggleStreamingButton_);
    controlLayout->addStretch();

    controlLayout->addWidget(rightPanelToggle_);

    controlLayout->addSpacerItem(new QSpacerItem(12, 0, QSizePolicy::Fixed, QSizePolicy::Minimum));

    controlLayout->addWidget(settingsButton_);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(videoView_, 1);  // prend tout l'espace
    contentLayout->addWidget(displayBar_, 0); // largeur naturelle

    // Assemblage global
    vLayout->addWidget(controlBar);
    vLayout->addLayout(contentLayout);

    setCentralWidget(central_);
}

void CameraWindow::setupConnections()
{
    // --- User actions ---

    connect(toggleStreamingButton_, &QPushButton::clicked, this, &CameraWindow::onToggleStreaming);

    connect(rightPanelToggle_, &QPushButton::toggled, displayBar_,
            &DisplaySettingsWidget::setPanelVisible);

    connect(settingsButton_, &QPushButton::clicked, cameraSettingsWindow_,
            &CameraSettingsWindow::show);

    // --- Hardware events (camera devices) ---

    connect(mediaDevices_, &QMediaDevices::videoInputsChanged, this,
            &CameraWindow::updateCameraList);

    connect(cameraController_, &CameraController::cameraStarted, this,
            &CameraWindow::onCameraStarted);

    connect(cameraController_, &CameraController::cameraStopped, this,
            &CameraWindow::onCameraStopped);

    connect(cameraController_, &CameraController::cameraError, this, &CameraWindow::onCameraError);

    // --- Controller → View / Window updates ---

    connect(cameraController_, &CameraController::frameSizeStr, this,
            &CameraWindow::onFrameSizeStr);

    connect(cameraController_, &CameraController::textStatsUpdated, videoView_,
            &ImageView::setText);

    // ---  View → Controller for display stats ---

    connect(videoView_, &ImageView::frameDisplayed, cameraController_,
            &CameraController::onFrameDisplayed);

    // --- Application settings synchronization ---

    bindApplicationSettingsToController();
    bindApplicationSettingsToView();
    bindUiToApplicationSettings();

    const auto& app = ApplicationSettings::instance();

    // refresh widget in function of settings
    connect(&app, &ApplicationSettings::videoSettingsChanged, this,
            [this](const VideoSessionSettings& conf)
            {
                bool hasPreprocessing =
                    conf.compute.downscale.hasDownscale || conf.compute.hasTemporalFiltering;

                displayBar_->updatePipelineAvailability(hasPreprocessing);
            });
}

void CameraWindow::applyInitialSettings()
{
    const auto& app = ApplicationSettings::instance();

    videoView_->applyDownscaleConfig(app.videoSettings().compute.downscale);
    videoView_->applyDisplayConfig(app.videoSettings().display);

    bool preprocessing = app.videoSettings().compute.downscale.hasDownscale ||
                         app.videoSettings().compute.hasTemporalFiltering;

    displayBar_->updatePipelineAvailability(preprocessing);

    updateCameraList();
}

void CameraWindow::bindApplicationSettingsToController()
{
    assert(cameraController_);

    const auto& app = ApplicationSettings::instance();

    connect(&app, &ApplicationSettings::videoSettingsChanged, cameraController_,
            &CameraController::onVideoSettingsChanged);

    connect(&app, &ApplicationSettings::videoDisplaySettingsChanged, cameraController_,
            &CameraController::onVideoDisplaySettingsChanged);
}

void CameraWindow::bindApplicationSettingsToView()
{
    assert(videoView_);

    const auto& app = ApplicationSettings::instance();

    connect(&app, &ApplicationSettings::videoSettingsChanged, this,
            [this](const VideoSessionSettings& conf)
            {
                videoView_->applyDownscaleConfig(conf.compute.downscale);
            });

    connect(&app, &ApplicationSettings::videoDisplaySettingsChanged, videoView_,
            &ImageView::applyDisplayConfig);
}

void CameraWindow::bindUiToApplicationSettings()
{
    assert(displayBar_ && cameraSettingsWindow_);

    const auto& app = ApplicationSettings::instance();

    connect(displayBar_, &DisplaySettingsWidget::displayConfigChanged, &app,
            &ApplicationSettings::setVideoDisplayConfig);

    // commit settings
    connect(cameraSettingsWindow_, &CameraSettingsWindow::videoSessionSettingsAccepted, &app,
            &ApplicationSettings::setVideoSessionSettings);
}

void CameraWindow::onFrameSizeStr(const QString& str)
{
    setWindowTitle(deviceWindowTitle_ + str);
}

void CameraWindow::updateCameraList()
{
    assert(mediaDevices_ && cameraSelector_ && toggleStreamingButton_);

    QSignalBlocker blocker(cameraSelector_);

    const auto cameras = mediaDevices_->videoInputs();

    QByteArray newlyAddedCamera;
    QList<QByteArray> currentIds;

    cameraSelector_->clear();

    for (const auto& cam : cameras)
    {
        currentIds.append(cam.id());

        if (!knownCameraIds_.contains(cam.id()))
            newlyAddedCamera = cam.id();

        const QIcon icon = (cam.id() == activeCameraId_) ? activeCameraIcon_ : emptyCameraIcon_;

        cameraSelector_->addItem(icon, cam.description(), cam.id());
    }

    // mémoriser la nouvelle liste
    knownCameraIds_ = currentIds;

    const bool hasCamera = !cameras.isEmpty();
    cameraSelector_->setEnabled(hasCamera);
    toggleStreamingButton_->setEnabled(hasCamera);

    if (!hasCamera)
    {
        cameraSelector_->setCurrentIndex(-1);
        return;
    }

    int index = -1;

    // 1️⃣ caméra active
    if (!activeCameraId_.isEmpty())
        index = cameraSelector_->findData(activeCameraId_);

    // 2️⃣ caméra nouvellement branchée
    if (index < 0 && !newlyAddedCamera.isEmpty())
        index = cameraSelector_->findData(newlyAddedCamera);

    // 3️⃣ caméra sauvegardée
    if (index < 0)
    {
        QByteArray savedId = loadSelectedCameraId();
        index = cameraSelector_->findData(savedId);
    }

    // 4️⃣ fallback
    if (index < 0)
        index = 0;

    cameraSelector_->setCurrentIndex(index);
}

void CameraWindow::onToggleStreaming()
{
    assert(cameraController_);

    if (cameraController_->isActive())
        stopCamera();
    else
        startCamera();
}

void CameraWindow::showEvent(QShowEvent* event)
{
    emit cameraWindowShown();
    QMainWindow::showEvent(event);
}

void CameraWindow::closeEvent(QCloseEvent* event)
{
    stopCamera();

    saveSelectedCameraId();

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
    frameToViewConnection_ = connect(cameraController_, &CameraController::imageAndContourUpdated,
                                     videoView_, &ImageView::setImageAndContour);
}

void CameraWindow::startCamera()
{
    const QByteArray selectedId = cameraSelector_->currentData().toByteArray();
    if (selectedId.isEmpty())
        return;

    cameraController_->start(selectedId);
}

void CameraWindow::stopCamera()
{
    if (cameraController_->isActive())
        cameraController_->stop();
}

void CameraWindow::onCameraStarted(const QByteArray& deviceId)
{
    activeCameraId_ = deviceId;

    videoView_->showPlaceholder(false);
    connectFrameToView();

    int index = cameraSelector_->findData(deviceId);
    if (index >= 0)
        cameraSelector_->setItemIcon(index, activeCameraIcon_);

    toggleStreamingButton_->setText(tr("Stop"));
    toggleStreamingButton_->setToolTip(tr("Stop camera streaming."));
    toggleStreamingButton_->setIcon(stopIcon_);

    saveSelectedCameraId();
}

void CameraWindow::onCameraStopped()
{
    activeCameraId_.clear();

    disconnect(frameToViewConnection_);
    videoView_->showPlaceholder(true);

    for (int i = 0; i < cameraSelector_->count(); ++i)
        cameraSelector_->setItemIcon(i, emptyCameraIcon_);

    toggleStreamingButton_->setText(tr("Start"));
    toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
    toggleStreamingButton_->setIcon(startIcon_);

    setWindowTitle(tr("Fluvel - Camera"));
}

void CameraWindow::onCameraError(const QByteArray& deviceId, QCamera::Error,
                                 const QString& errorString)
{
    disconnect(frameToViewConnection_);
    videoView_->showPlaceholder(true);

    QMessageBox::warning(this, tr("Camera error"), errorString);

    int index = cameraSelector_->findData(deviceId);
    if (index >= 0)
        cameraSelector_->setItemIcon(index, errorCameraIcon_);
}

static constexpr auto kCameraDeviceKey = "camera/device";

QByteArray CameraWindow::loadSelectedCameraId()
{
    QSettings settings;
    return settings.value(kCameraDeviceKey).toByteArray();
}

void CameraWindow::saveSelectedCameraId()
{
    QSettings settings;
    settings.setValue(kCameraDeviceKey, cameraSelector_->currentData().toByteArray());
}

} // namespace fluvel_app
