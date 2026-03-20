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

    activeCameraIcon_ = createActiveCameraIcon();
    emptyCameraIcon_ = createEmptyCameraIcon();
    errorCameraIcon_ = createErrorCameraIcon();

    startIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart, QStyle::SP_MediaPlay,
                              ":/icons/toolbar/media-playback-start-symbolic.svg");

    stopIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop, QStyle::SP_MediaStop,
                             ":/icons/toolbar/media-playback-stop-symbolic.svg");

    toggleStreamingButton_ = new QPushButton;

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

    imageViewer_ = new ImageViewerWidget(app.videoSettings().display,
                                         app.videoSettings().compute.downscale, central_);

    imageViewer_->setMaxDisplayFps(30.0);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    imageViewer_->setInteraction(interaction.release());
}

void CameraWindow::setupController()
{
    const auto& config = ApplicationSettings::instance().videoSettings();
    cameraController_ = new CameraController(config, this);
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

    contentLayout->addWidget(imageViewer_, 1); // prend tout l'espace
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

    connect(cameraController_, &CameraController::videoInputsChanged, this,
            &CameraWindow::updateCameraList);

    connect(cameraController_, &CameraController::streamingStarted, this,
            &CameraWindow::onStreamingStarted);

    connect(cameraController_, &CameraController::streamingStopped, this,
            &CameraWindow::onStreamingStopped);

    connect(cameraController_, &CameraController::cameraError, this, &CameraWindow::onCameraError);

    connect(cameraController_, &CameraController::streamingLost, this,
            &CameraWindow::onStreamingLost);

    connect(cameraController_, &CameraController::startupTimeout, this,
            &CameraWindow::onStartupTimeout);

    // --- Controller → View / Window updates ---

    connect(cameraController_, &CameraController::frameSizeStr, this,
            &CameraWindow::onFrameSizeStr);

    connect(cameraController_, &CameraController::textStatsUpdated, imageViewer_,
            &ImageViewerWidget::setText);

    // ---  View → Controller for display stats ---

    connect(imageViewer_, &ImageViewerWidget::frameDisplayed, cameraController_,
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

    // refresh button in function of user action
    connect(cameraSelector_, &QComboBox::currentIndexChanged, this,
            &CameraWindow::updateStreamingButton);
}

void CameraWindow::applyInitialSettings()
{
    const auto& app = ApplicationSettings::instance();

    imageViewer_->applyDownscaleConfig(app.videoSettings().compute.downscale);
    imageViewer_->applyDisplayConfig(app.videoSettings().display);

    bool preprocessing = app.videoSettings().compute.downscale.hasDownscale ||
                         app.videoSettings().compute.hasTemporalFiltering;

    displayBar_->updatePipelineAvailability(preprocessing);

    refreshUi();
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
    assert(imageViewer_);

    const auto& app = ApplicationSettings::instance();

    connect(&app, &ApplicationSettings::videoSettingsChanged, this,
            [this](const VideoSessionSettings& conf)
            {
                imageViewer_->applyDownscaleConfig(conf.compute.downscale);
            });

    connect(&app, &ApplicationSettings::videoDisplaySettingsChanged, imageViewer_,
            &ImageViewerWidget::applyDisplayConfig);
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

void CameraWindow::updateCameraList(const QList<QCameraDevice>& inputs)
{
    assert(cameraSelector_ && toggleStreamingButton_);

    QSignalBlocker blocker(cameraSelector_);

    const auto cameras = inputs;

    QByteArray newlyAddedCamera{};
    QSet<QByteArray> currentIds;

    const QByteArray previousSelection = cameraSelector_->currentData().toByteArray();
    cameraSelector_->clear();

    for (const auto& cam : cameras)
    {
        const auto camId = cam.id();

        currentIds.insert(camId);

        if (!knownCameraIds_.isEmpty() && !knownCameraIds_.contains(camId))
            newlyAddedCamera = camId;

        QIcon icon = emptyCameraIcon_;

        const auto state = cameraStatus_.value(camId, CameraStatus::Normal);

        if (state == CameraStatus::Active)
            icon = activeCameraIcon_;
        else if (state == CameraStatus::Error)
            icon = errorCameraIcon_;

        cameraSelector_->addItem(icon, cam.description(), camId);
    }

    knownCameraIds_ = currentIds;

    const bool hasCamera = !cameras.isEmpty();

    setCameraControlsEnabled(hasCamera);

    int currentIndex = -1;

    if (hasCamera)
        currentIndex = computeBestCameraIndex(previousSelection, newlyAddedCamera);

    cameraSelector_->setCurrentIndex(currentIndex);

    emit cameraAvailabilityChanged(isCameraAvailable());
}

int CameraWindow::computeBestCameraIndex(const QByteArray& previousSelection,
                                         const QByteArray& newlyPlugged)
{
    assert(cameraSelector_);

    int index = -1;

    // 1 newly plugged camera
    if (index < 0 && !newlyPlugged.isEmpty())
        index = cameraSelector_->findData(newlyPlugged);

    // 2 user's previous selection
    if (index < 0 && !previousSelection.isEmpty())
        index = cameraSelector_->findData(previousSelection);

    // 3 active camera
    if (index < 0 && !streamingCameraId_.isEmpty())
        index = cameraSelector_->findData(streamingCameraId_);

    // 4 saved camera
    if (index < 0)
        index = cameraSelector_->findData(loadSelectedCameraId());

    // 5 fallback
    if (index < 0)
        index = 0;

    return index;
}

void CameraWindow::setCameraControlsEnabled(bool enabled)
{
    assert(cameraSelector_ && toggleStreamingButton_);

    cameraSelector_->setEnabled(enabled);
    toggleStreamingButton_->setEnabled(enabled);
}

void CameraWindow::onToggleStreaming()
{
    assert(cameraSelector_ && cameraController_);

    if (cameraSelector_->currentIndex() < 0)
        return;

    QByteArray selected = cameraSelector_->currentData().toByteArray();

    if (!cameraController_->isStreaming())
    {
        startCamera();
    }
    else if (selected == streamingCameraId_)
    {
        stopCamera();
    }
    else
    {
        switchingInProgress_ = true;
        stopCamera();
        startCamera();
    }
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
    disconnect(frameToViewConnection_);
    frameToViewConnection_ = connect(cameraController_, &CameraController::imageAndContourUpdated,
                                     imageViewer_, &ImageViewerWidget::setImageAndContour);
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
    cameraController_->stop();
}

void CameraWindow::onStreamingStarted(const QByteArray& deviceId)
{
    streamingCameraId_ = deviceId;
    cameraStatus_[deviceId] = CameraStatus::Active;

    if (!switchingInProgress_)
    {
        imageViewer_->showPlaceholder(false);
        connectFrameToView();
    }

    refreshUi();
    saveSelectedCameraId();

    switchingInProgress_ = false;
}

void CameraWindow::onStreamingStopped()
{
    if (!streamingCameraId_.isEmpty())
    {
        if (cameraStatus_[streamingCameraId_] == CameraStatus::Active)
            cameraStatus_[streamingCameraId_] = CameraStatus::Normal;
    }

    streamingCameraId_.clear();

    if (!switchingInProgress_)
    {
        disconnect(frameToViewConnection_);
        imageViewer_->showPlaceholder(true);

        refreshUi();
    }

    setWindowTitle(tr("Fluvel - Camera"));
}

void CameraWindow::onCameraError(const QByteArray& deviceId, QCamera::Error,
                                 const QString& errorString)
{
    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QMessageBox::warning(this, tr("Camera error"), errorString);

    cameraStatus_[deviceId] = CameraStatus::Error;

    // un switch raté devient un stop
    if (switchingInProgress_)
    {
        switchingInProgress_ = false;
        streamingCameraId_.clear();
    }

    refreshUi();
}

void CameraWindow::onStartupTimeout(const QByteArray& deviceId, double timeoutSec)
{
    QMessageBox::warning(this, tr("Camera startup failed"),
                         tr("The camera did not produce a valid frame within %1 seconds.\n"
                            "The device may be busy or not responding.")
                             .arg(timeoutSec, 0, 'f', 1));

    cameraStatus_[deviceId] = CameraStatus::Error;

    refreshUi();
}

void CameraWindow::onStreamingLost(const QByteArray& deviceId, double frameAgeSec)
{
    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QMessageBox::warning(
        this, tr("Camera stream lost"),
        tr("No valid frame received for %1 seconds.\nThe camera stream may have stalled.")
            .arg(frameAgeSec, 0, 'f', 1));

    cameraStatus_[deviceId] = CameraStatus::Error;

    refreshUi();
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

void CameraWindow::updateStreamingButton()
{
    if (!cameraController_->isStreaming())
    {
        toggleStreamingButton_->setText(tr("Start"));
        toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
        toggleStreamingButton_->setIcon(startIcon_);
        return;
    }

    QByteArray selected = cameraSelector_->currentData().toByteArray();

    if (selected == streamingCameraId_)
    {
        toggleStreamingButton_->setText(tr("Stop"));
        toggleStreamingButton_->setToolTip(tr("Stop camera streaming."));
        toggleStreamingButton_->setIcon(stopIcon_);
    }
    else
    {
        toggleStreamingButton_->setText(tr("Switch"));
        toggleStreamingButton_->setToolTip(tr("Switch to selected camera."));
        toggleStreamingButton_->setIcon(startIcon_);
    }
}

void CameraWindow::refreshUi()
{
    assert(cameraController_);

    updateCameraList(cameraController_->videoInputs());
    updateStreamingButton();
}

bool CameraWindow::isCameraAvailable() const
{
    assert(cameraSelector_);

    return cameraSelector_->count() > 0;
}

} // namespace fluvel_app
