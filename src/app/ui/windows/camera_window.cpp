// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_window.hpp"
#include "application_settings.hpp"
#include "autofit_behavior.hpp"
#include "camera_controller.hpp"
#include "camera_format_utils.hpp"
#include "camera_settings_window.hpp"
#include "device_id_utils.hpp"
#include "display_settings_widget.hpp"
#include "fullscreen_behavior.hpp"
#include "icon_loader.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "qcolor_utils.hpp"
#include "right_panel_toggle_button.hpp"

#include <QCameraDevice>
#include <QComboBox>
#include <QLabel>
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

    loadPreferredFormats();

    applyInitialSettings();
    setupConnections();
}

void CameraWindow::setupWindow()
{
    setWindowIcon(QIcon(":/icons/app/fluvel.svg"));
    updateWindowTitle();
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

    deviceLabel_ = new QLabel(tr("Device:"));
    deviceSelector_ = new QComboBox(this);

    // Adjust width to contents (needed when items have icons)
    deviceSelector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    static constexpr int kCameraIconSize = 13;
    deviceSelector_->setIconSize(QSize(kCameraIconSize, kCameraIconSize));

    deviceActiveIcon_ = createActiveCameraIcon();
    deviceIdleIcon_ = createEmptyIcon(kCameraIconSize);
    deviceErrorIcon_ = createErrorCameraIcon();

    formatLabel_ = new QLabel(tr("Format:"));
    formatSelector_ = new QComboBox(this);
    formatSelector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    static constexpr int kFormatIconSize = 16;
    formatSelector_->setIconSize(QSize(kFormatIconSize, kFormatIconSize));

    formatActiveIcon_ = createActiveFormatIcon();
    formatAvailableIcon_ = createEmptyIcon(kFormatIconSize);

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

    p.setBrush(QColor(0, 180, 0));
    p.setPen(QPen(QColor(40, 40, 40), 1));

    p.drawEllipse(1, 1, ellipseSize, ellipseSize);

    return QIcon(pix);
}

QIcon CameraWindow::createEmptyIcon(int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    return QIcon(pix);
}

QIcon CameraWindow::createActiveFormatIcon()
{
    return il::loadIcon("emblem-default", ":/icons/toolbar/check-symbolic.svg");
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
    QVBoxLayout* vLayout = new QVBoxLayout(central_);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    QWidget* controlBar = new QWidget(central_);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar); // 👈 direct en horizontal
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(8);

    // Device
    controlLayout->addWidget(deviceLabel_);
    controlLayout->addWidget(deviceSelector_);

    // Format
    controlLayout->addSpacing(12);
    controlLayout->addWidget(formatLabel_);
    controlLayout->addWidget(formatSelector_);

    // Action principale
    controlLayout->addSpacing(12);
    controlLayout->addWidget(toggleStreamingButton_);

    // Stretch → pousse le reste à droite
    controlLayout->addStretch();

    // Actions secondaires
    controlLayout->addWidget(rightPanelToggle_);
    controlLayout->addSpacing(8);
    controlLayout->addWidget(settingsButton_);

    // --- Layout contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(imageViewer_, 1);
    contentLayout->addWidget(displayBar_, 0);

    // Assemblage
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

    connect(deviceSelector_, &QComboBox::currentIndexChanged, this, &CameraWindow::onDeviceChanged);

    connect(formatSelector_, &QComboBox::currentIndexChanged, this,
            [this]()
            {
                if (isUpdatingUi_)
                    return;

                auto fmt = getSelectedFormat();
                if (!fmt.isNull() && !selectedDeviceId_.isEmpty())
                {
                    preferredFormats_[selectedDeviceId_] = fmt;
                    savePreferredFormats();
                }
            });

    // --- Hardware events (camera devices) ---

    connect(cameraController_, &CameraController::videoInputsChanged, this,
            &CameraWindow::updateDeviceList);

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
    connect(deviceSelector_, &QComboBox::currentIndexChanged, this,
            &CameraWindow::updateStreamingButton);

    connect(cameraController_, &CameraController::downscaleChanged, this,
            &CameraWindow::onDownscaleChanged);
}

void CameraWindow::applyInitialSettings()
{
    assert(imageViewer_ && displayBar_);

    const auto& app = ApplicationSettings::instance();
    const auto& downscaleConfig = app.videoSettings().compute.downscale;

    imageViewer_->applyDownscaleConfig(downscaleConfig);
    imageViewer_->applyDisplayConfig(app.videoSettings().display);

    bool preprocessing = app.videoSettings().compute.downscale.hasDownscale ||
                         app.videoSettings().compute.hasTemporalFiltering;

    displayBar_->updatePipelineAvailability(preprocessing);

    refreshUi();

    onDownscaleChanged(downscaleConfig);
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

void CameraWindow::updateDeviceList(const QList<QCameraDevice>& devices)
{
    assert(deviceSelector_);

    ScopedUiUpdateGuard guard(isUpdatingUi_);

    QSignalBlocker blocker(deviceSelector_);

    QByteArray newlyAddedCamera{};
    QSet<QByteArray> currentIds;

    const QByteArray previousSelection = deviceSelector_->currentData().toByteArray();
    deviceSelector_->clear();

    for (const auto& dev : devices)
    {
        const auto deviceId = dev.id();

        currentIds.insert(deviceId);

        if (!knownDeviceIds_.isEmpty() && !knownDeviceIds_.contains(deviceId))
            newlyAddedCamera = deviceId;

        QIcon icon = deviceIdleIcon_;

        const auto state = deviceStreamingStatus_.value(deviceId, DeviceStreamingStatus::Idle);

        if (state == DeviceStreamingStatus::Streaming)
            icon = deviceActiveIcon_;
        else if (state == DeviceStreamingStatus::Error)
            icon = deviceErrorIcon_;

        deviceSelector_->addItem(icon, dev.description(), deviceId);
    }

    knownDeviceIds_ = currentIds;

    const bool hasDevice = !devices.isEmpty();

    setDeviceControlsEnabled(hasDevice);

    int currentIndex = -1;

    if (hasDevice)
        currentIndex = computeBestDeviceIndex(previousSelection, newlyAddedCamera);

    deviceSelector_->setCurrentIndex(currentIndex);

    if (currentIndex >= 0)
        onDeviceChanged(currentIndex);

    emit cameraAvailabilityChanged(isCameraAvailable());
}

int CameraWindow::computeBestDeviceIndex(const QByteArray& previousSelection,
                                         const QByteArray& newlyPlugged)
{
    assert(deviceSelector_);

    int index = -1;

    // 1 newly plugged camera
    if (index < 0 && !newlyPlugged.isEmpty())
        index = deviceSelector_->findData(newlyPlugged);

    // 2 user's previous selection
    if (index < 0 && !previousSelection.isEmpty())
        index = deviceSelector_->findData(previousSelection);

    // 3 active camera
    if (index < 0 && !streamingDeviceId_.isEmpty())
        index = deviceSelector_->findData(streamingDeviceId_);

    // 4 saved camera
    if (index < 0)
        index = deviceSelector_->findData(loadSelectedCameraId());

    // 5 fallback
    if (index < 0)
        index = 0;

    return index;
}

void CameraWindow::setDeviceControlsEnabled(bool enabled)
{
    assert(deviceSelector_ && toggleStreamingButton_);

    deviceSelector_->setEnabled(enabled);
    toggleStreamingButton_->setEnabled(enabled);
}

void CameraWindow::onToggleStreaming()
{
    assert(deviceSelector_ && cameraController_);

    if (deviceSelector_->currentIndex() < 0)
        return;

    QByteArray selected = deviceSelector_->currentData().toByteArray();

    if (!cameraController_->isStreaming())
    {
        startCamera();
    }
    else if (selected == streamingDeviceId_)
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

void CameraWindow::onDeviceChanged(int /*index*/)
{
    refreshFormatListFromSelection();
}

void CameraWindow::refreshFormatListFromSelection()
{
    assert(deviceSelector_ && formatSelector_ && cameraController_);

    ScopedUiUpdateGuard guard(isUpdatingUi_);

    int index = deviceSelector_->currentIndex();
    if (index < 0)
    {
        formatSelector_->clear();
        return;
    }

    QByteArray deviceId = deviceSelector_->itemData(index).toByteArray();
    selectedDeviceId_ = deviceId;

    // 👉 récupérer directement depuis la liste actuelle
    const auto devices = cameraController_->videoInputs();

    const QCameraDevice* device = nullptr;

    for (const auto& d : devices)
    {
        if (d.id() == deviceId)
        {
            device = &d;
            break;
        }
    }

    if (!device)
    {
        formatSelector_->clear();
        return;
    }

    updateFormatList(device->videoFormats());
}

void CameraWindow::updateFormatList(const QList<QCameraFormat>& formats)
{
    assert(formatSelector_);

    ScopedUiUpdateGuard guard(isUpdatingUi_);
    QSignalBlocker blocker(formatSelector_);

    formatSelector_->clear();

    int bestFormatIndex = findBestFormatIndex(formats);

    int indexToSelect = -1;

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& fmt = formats[i];

        bool isActive = camera_utils::isSameCameraFormat(fmt, activeFormat_);
        bool isRecommended = (i == bestFormatIndex);

        QString label = formatToString(fmt);

        if (isRecommended)
            label += "  *";

        formatSelector_->addItem(isActive ? formatActiveIcon_ : formatAvailableIcon_, label,
                                 QVariant::fromValue(fmt));

        if (isRecommended)
            formatSelector_->setItemData(i, tr("Recommended format"), Qt::ToolTipRole);
    }

    // 1. PRIORITÉ : format associé à CE device uniquement
    if (!selectedDeviceId_.isEmpty())
    {
        auto preferred = preferredFormats_.value(selectedDeviceId_);

        if (!preferred.isNull())
        {
            for (int i = 0; i < formats.size(); ++i)
            {
                if (camera_utils::isSameCameraFormat(formats[i], preferred))
                {
                    indexToSelect = i;
                    break;
                }
            }
        }
    }

    // 2. fallback best format
    if (indexToSelect < 0 && !formats.isEmpty())
        indexToSelect = bestFormatIndex;

    // application
    if (indexToSelect < 0 && formatSelector_->count() > 0)
        indexToSelect = 0;

    if (indexToSelect >= 0)
        formatSelector_->setCurrentIndex(indexToSelect);
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
    savePreferredFormats();

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
    assert(deviceSelector_ && cameraController_);

    if (deviceSelector_->currentIndex() < 0)
        return;

    QByteArray selectedId = deviceSelector_->currentData().toByteArray();
    if (selectedId.isEmpty())
        return;

    const auto selectedFormat = getSelectedFormat();

    cameraController_->start(selectedId, selectedFormat);
}

QCameraFormat CameraWindow::getSelectedFormat() const
{
    assert(formatSelector_);

    int index = formatSelector_->currentIndex();

    if (index < 0)
        return QCameraFormat();

    return formatSelector_->itemData(index).value<QCameraFormat>();
}

void CameraWindow::stopCamera()
{
    assert(cameraController_);

    cameraController_->stop();
}

void CameraWindow::onStreamingStarted(const StreamingInfo& info)
{
    assert(imageViewer_);

    streamingDeviceId_ = info.deviceId;
    deviceStreamingStatus_[info.deviceId] = DeviceStreamingStatus::Streaming;
    preferredFormats_[info.deviceId] = info.format;

    activeFormat_ = info.format;

    if (!switchingInProgress_)
    {
        imageViewer_->showPlaceholder(false);
        connectFrameToView();
    }

    refreshUi();

    deviceTitleStr_ =
        QString("Fluvel — %1 - %2").arg(info.description, formatToString(activeFormat_));

    updateWindowTitle();

    saveSelectedCameraId();
    savePreferredFormats();

    switchingInProgress_ = false;
}

void CameraWindow::onStreamingStopped()
{
    assert(imageViewer_);

    if (!streamingDeviceId_.isEmpty())
    {
        if (deviceStreamingStatus_[streamingDeviceId_] == DeviceStreamingStatus::Streaming)
            deviceStreamingStatus_[streamingDeviceId_] = DeviceStreamingStatus::Idle;
    }

    streamingDeviceId_.clear();
    activeFormat_ = QCameraFormat();

    if (!switchingInProgress_)
    {
        disconnect(frameToViewConnection_);
        imageViewer_->showPlaceholder(true);

        refreshUi();
    }

    updateWindowTitle();
}

void CameraWindow::onCameraError(const QByteArray& deviceId, QCamera::Error,
                                 const QString& errorString)
{
    assert(imageViewer_);

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QMessageBox::warning(this, tr("Camera error"), errorString);

    deviceStreamingStatus_[deviceId] = DeviceStreamingStatus::Error;

    // un switch raté devient un stop
    if (switchingInProgress_)
    {
        switchingInProgress_ = false;
        streamingDeviceId_.clear();
    }

    refreshUi();
}

void CameraWindow::onStartupTimeout(const QByteArray& deviceId, double timeoutSec)
{
    QMessageBox::warning(this, tr("Camera startup failed"),
                         tr("The camera did not produce a valid frame within %1 seconds.\n"
                            "The device may be busy or not responding.")
                             .arg(timeoutSec, 0, 'f', 1));

    deviceStreamingStatus_[deviceId] = DeviceStreamingStatus::Error;

    refreshUi();
}

void CameraWindow::onStreamingLost(const QByteArray& deviceId, double frameAgeSec)
{
    assert(imageViewer_);

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QMessageBox::warning(
        this, tr("Camera stream lost"),
        tr("No valid frame received for %1 seconds.\nThe camera stream may have stalled.")
            .arg(frameAgeSec, 0, 'f', 1));

    deviceStreamingStatus_[deviceId] = DeviceStreamingStatus::Error;

    refreshUi();
}

void CameraWindow::updateStreamingButton()
{
    assert(cameraController_ && toggleStreamingButton_ && deviceSelector_);

    if (!cameraController_->isStreaming())
    {
        toggleStreamingButton_->setText(tr("Start"));
        toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
        toggleStreamingButton_->setIcon(startIcon_);
        return;
    }

    QByteArray selected = deviceSelector_->currentData().toByteArray();

    if (selected == streamingDeviceId_)
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

    updateDeviceList(cameraController_->videoInputs());
    updateStreamingButton();
}

bool CameraWindow::isCameraAvailable() const
{
    assert(deviceSelector_);

    return deviceSelector_->count() > 0;
}

QString CameraWindow::pixelFormatToShortString(QVideoFrameFormat::PixelFormat fmt)
{
    switch (fmt)
    {
        case QVideoFrameFormat::Format_NV12:
            return "NV12";
        case QVideoFrameFormat::Format_NV21:
            return "NV21";
        case QVideoFrameFormat::Format_YUV420P:
            return "YUV420";
        case QVideoFrameFormat::Format_YUYV:
            return "YUYV";
        case QVideoFrameFormat::Format_Jpeg:
            return "MJPEG";
        default:
            return "Other";
    }
}

QString CameraWindow::formatToString(const QCameraFormat& fmt)
{
    const QSize res = fmt.resolution();
    const int fps = static_cast<int>(fmt.maxFrameRate());

    const QString pixelFormat = pixelFormatToShortString(fmt.pixelFormat());

    return QString("%1 %2x%3 @%4").arg(pixelFormat).arg(res.width()).arg(res.height()).arg(fps);
}

int CameraWindow::findBestFormatIndex(const QList<QCameraFormat>& formats) const
{
    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv420(f.pixelFormat()) && is640x480(f.resolution()) && is30fps(f.maxFrameRate()))
            return i;
    }

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv(f.pixelFormat()) && is640x480(f.resolution()) && is30fps(f.maxFrameRate()))
            return i;
    }

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv(f.pixelFormat()) && is640x480(f.resolution()))
            return i;
    }

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv(f.pixelFormat()))
            return i;
    }

    return -1;
}

bool CameraWindow::isYuv420(QVideoFrameFormat::PixelFormat fmt) const
{
    return fmt == QVideoFrameFormat::Format_YUV420P || fmt == QVideoFrameFormat::Format_NV12 ||
           fmt == QVideoFrameFormat::Format_NV21;
}

bool CameraWindow::isYuv(QVideoFrameFormat::PixelFormat fmt) const
{
    return isYuv420(fmt) || fmt == QVideoFrameFormat::Format_YUYV;
}

bool CameraWindow::is640x480(const QSize& s) const
{
    return s.width() == 640 && s.height() == 480;
}

bool CameraWindow::is30fps(float fps) const
{
    return std::abs(fps - 30.0f) < 1.0f;
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
    settings.setValue(kCameraDeviceKey, deviceSelector_->currentData().toByteArray());
}

static constexpr auto kCameraFormatsKey = "camera/formats";

void CameraWindow::savePreferredFormats()
{
    QSettings settings;

    settings.beginGroup(kCameraFormatsKey);
    settings.remove(""); // reset

    for (auto it = preferredFormats_.begin(); it != preferredFormats_.end(); ++it)
    {
        if (it.value().isNull())
            continue;

        const QString key = device::encodeDeviceId(it.key());
        const QCameraFormat& fmt = it.value();

        settings.beginGroup(key);
        settings.setValue("w", fmt.resolution().width());
        settings.setValue("h", fmt.resolution().height());
        settings.setValue("fps", fmt.maxFrameRate());
        settings.setValue("pf", static_cast<int>(fmt.pixelFormat()));
        settings.endGroup();
    }

    settings.endGroup();
}

void CameraWindow::loadPreferredFormats()
{
    assert(cameraController_);

    QSettings settings;

    settings.beginGroup(kCameraFormatsKey);

    const auto devices = settings.childGroups();

    for (const QString& dev : devices)
    {
        settings.beginGroup(dev);

        QSize res(settings.value("w").toInt(), settings.value("h").toInt());

        float fps = settings.value("fps").toFloat();

        auto pf = static_cast<QVideoFrameFormat::PixelFormat>(settings.value("pf").toInt());

        settings.endGroup();

        // 👉 on ne peut pas reconstruire directement un QCameraFormat
        // donc on stocke une "cible" temporaire et on match plus tard

        for (const auto& cam : cameraController_->videoInputs())
        {
            QByteArray deviceId = device::decodeDeviceId(dev);

            if (cam.id() != deviceId)
                continue;

            for (const auto& fmt : cam.videoFormats())
            {
                if (fmt.resolution() == res && fmt.pixelFormat() == pf &&
                    std::abs(fmt.maxFrameRate() - fps) < 2.0f)
                {
                    preferredFormats_[deviceId] = fmt;
                    break;
                }
            }
        }
    }

    settings.endGroup();
}

void CameraWindow::onDownscaleChanged(const DownscaleConfig& downscaleConfig)
{
    assert(cameraController_);

    downscaleTitleStr_.clear();

    if (downscaleConfig.hasDownscale)
        downscaleTitleStr_ = QString("(/%1)").arg(downscaleConfig.downscaleFactor);

    updateWindowTitle();
}

void CameraWindow::updateWindowTitle()
{
    if (cameraController_ && cameraController_->isStreaming())
    {
        QString title = deviceTitleStr_;

        if (!downscaleTitleStr_.isEmpty())
            title += " " + downscaleTitleStr_;

        setWindowTitle(title);
    }
    else
    {
        setWindowTitle(tr("Fluvel — Camera"));
    }
}

} // namespace fluvel_app
