// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_window.hpp"
#include "application_settings.hpp"
#include "autofit_behavior.hpp"
#include "camera_format_utils.hpp"
#include "device_id_utils.hpp"
#include "display_settings_widget.hpp"
#include "file_utils.hpp"
#include "fullscreen_behavior.hpp"
#include "icon_loader.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "qcolor_utils.hpp"
#include "right_panel_toggle_button.hpp"
#include "video_controller.hpp"
#include "video_settings_window.hpp"
#include "video_types.hpp"

#include <QCameraDevice>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

#include <utility>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QPermission>
#include <QtCore/qpermissions.h>
#endif

static constexpr auto kLastSourceTypeKey = "sources/last_type";
static constexpr auto kCameraDeviceKey = "camera/device";
static constexpr auto kCameraFormatsKey = "camera/formats";
static constexpr auto kSourceHistoryKey = "sources/history";
static constexpr auto kLastVideoDirectory = "video/last_directory";

namespace fluvel
{

VideoWindow::VideoWindow(QWidget* parent)
    : QMainWindow(parent)

{
    setupWindow();
    restoreSettings();

    createUi();
    setupView();
    setupController();
    setupLayout();

    loadPreferredFormats();
    loadSourceHistory();

    applyInitialSettings();
    setupConnections();
}

void VideoWindow::setupWindow()
{
    setWindowIcon(il::appIcon());
    updateWindowTitle();
}

void VideoWindow::restoreSettings()
{
    QSettings settings;

    if (settings.contains("ui_geometry/camera_window"))
        restoreGeometry(settings.value("ui_geometry/camera_window").toByteArray());
    else
        resize(900, 600);
}

void VideoWindow::createUi()
{
    const auto& config = ApplicationSettings::instance().videoSettings();

    central_ = new QWidget(this);

    sourceLabel_ = new QLabel(tr("Source: "));

    sourceTypeCombo_ = new QComboBox(this);
    sourceTypeCombo_->addItem(tr("Camera"), QVariant::fromValue(SourceType::Camera));
    sourceTypeCombo_->addItem(tr("URL"), QVariant::fromValue(SourceType::Url));
    sourceTypeCombo_->addItem(tr("File"), QVariant::fromValue(SourceType::File));

    deviceLabel_ = new QLabel(tr("Device: "));
    deviceSelector_ = new QComboBox(this);

    // Adjust width to contents (needed when items have icons)
    deviceSelector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    static constexpr int kCameraIconSize{13};
    deviceSelector_->setIconSize(QSize(kCameraIconSize, kCameraIconSize));

    deviceActiveIcon_ = createActiveCameraIcon();
    deviceIdleIcon_ = createEmptyIcon(kCameraIconSize);
    deviceErrorIcon_ = createErrorCameraIcon();

    formatLabel_ = new QLabel(tr("Format: "));
    formatSelector_ = new QComboBox(this);
    formatSelector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    static constexpr int kFormatIconSize{16};
    formatSelector_->setIconSize(QSize(kFormatIconSize, kFormatIconSize));

    formatActiveIcon_ = createActiveFormatIcon();
    formatAvailableIcon_ = createEmptyIcon(kFormatIconSize);

    openFileButton_ = new QPushButton(tr("Open..."));
    openFileButton_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));

    sourceCombo_ = new QComboBox(this);
    sourceCombo_->setEditable(true);

    clearButton_ = new QPushButton(tr("Clear"));
    QIcon deleteIcon =
        il::loadIcon(QIcon::ThemeIcon::EditClear, ":/icons/actions/edit-clear-history.svg");

    clearButton_->setIcon(deleteIcon);

    startIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                              ":/icons/media/media-playback-start-symbolic.svg");

    stopIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                             ":/icons/media/media-playback-stop-symbolic.svg");

    toggleStreamingButton_ = new QPushButton;

    applyButton_ = new QPushButton;
    applyButton_->setVisible(false);
    applyButton_->setFlat(true);
    applyButton_->setText(tr("Apply"));
    applyButton_->setToolTip(tr("Apply selected camera and format."));

    QIcon applyIcon = createActiveFormatIcon();
    applyButton_->setIcon(applyIcon);

    rightPanelToggle_ = new RightPanelToggleButton;

    settingsButton_ = new QPushButton;
    settingsButton_->setToolTip(tr("Camera session settings"));
    settingsButton_->setFlat(true);
    settingsButton_->setFocusPolicy(Qt::NoFocus);

    settingsIcon_ = il::loadIcon("configure", ":/icons/actions/settings-symbolic.svg");

    settingsButton_->setIcon(settingsIcon_);

    // --- Display bar ---
    displayBar_ = new DisplaySettingsWidget(config.display, central_);

    videoSettingsWindow_ = new VideoSettingsWindow(config.compute, this);
}

QIcon VideoWindow::createActiveCameraIcon()
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

QIcon VideoWindow::createEmptyIcon(int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    return QIcon(pix);
}

QIcon VideoWindow::createActiveFormatIcon()
{
    QIcon icon;

#ifndef FLUVEL_PLATFORM_LINUX
    icon = il::loadIcon("emblem-default", ":/icons/actions/check-symbolic.svg");
#else
    icon = il::loadIcon(":/icons/status/check-symbolic.svg");
#endif

    return icon;
}

QIcon VideoWindow::createErrorCameraIcon()
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

void VideoWindow::setupView()
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

void VideoWindow::setupController()
{
    const auto& config = ApplicationSettings::instance().videoSettings();
    videoController_ = new VideoController(config, this);
}

void VideoWindow::setupLayout()
{
    QVBoxLayout* vLayout = new QVBoxLayout(central_);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    QWidget* controlBar = new QWidget(central_);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar); // 👈 direct en horizontal
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(8);

    // Device
    controlLayout->addWidget(sourceLabel_);
    controlLayout->addWidget(sourceTypeCombo_);

    // Device
    controlLayout->addWidget(deviceLabel_);
    controlLayout->addWidget(deviceSelector_);

    // Format
    controlLayout->addSpacing(12);
    controlLayout->addWidget(formatLabel_);
    controlLayout->addWidget(formatSelector_);

    // File/Url
    controlLayout->addWidget(openFileButton_);
    controlLayout->addWidget(sourceCombo_, 1);
    controlLayout->addWidget(clearButton_);

    // Action principale
    controlLayout->addSpacing(12);
    controlLayout->addWidget(toggleStreamingButton_);
    controlLayout->addWidget(applyButton_);

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

void VideoWindow::setupConnections()
{
    // --- User actions ---

    connect(sourceTypeCombo_, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                sourceConfig_.type = sourceTypeCombo_->itemData(index).value<SourceType>();

                saveLastSourceType();

                refreshSourceUi();
                updateApplyButton();
            });

    connect(deviceSelector_, &QComboBox::currentIndexChanged, this, &VideoWindow::onDeviceChanged);

    connect(formatSelector_, &QComboBox::currentIndexChanged, this,
            [this]()
            {
                if (isUpdatingUi_)
                    return;

                updateApplyButton();

                auto fmt = getSelectedFormat();

                if (!fmt.isNull() && !sourceConfig_.cameraId.isEmpty())
                {
                    sourceConfig_.cameraFormat = fmt;
                    savePreferredFormats();
                }
            });

    connect(openFileButton_, &QPushButton::clicked, this, &VideoWindow::openFile);

    connect(clearButton_, &QPushButton::clicked, this,
            [this]()
            {
                sourceCombo_->clear();
                saveSourceHistory();
            });

    connect(toggleStreamingButton_, &QPushButton::clicked, this, &VideoWindow::onToggleStreaming);

    connect(applyButton_, &QPushButton::clicked, this, &VideoWindow::onApplySelection);

    connect(rightPanelToggle_, &QPushButton::toggled, displayBar_,
            &DisplaySettingsWidget::setPanelVisible);

    connect(settingsButton_, &QPushButton::clicked, videoSettingsWindow_,
            &VideoSettingsWindow::show);

    // --- Hardware events (camera devices) ---

    connect(videoController_, &VideoController::videoInputsChanged, this,
            &VideoWindow::updateDeviceList);

    connect(videoController_, &VideoController::streamingStarting, this,
            &VideoWindow::onStreamingStarting);

    connect(videoController_, &VideoController::streamingStarted, this,
            &VideoWindow::onStreamingStarted);

    connect(videoController_, &VideoController::streamingStopped, this,
            &VideoWindow::onStreamingStopped);

    connect(videoController_, &VideoController::cameraError, this, &VideoWindow::onCameraError);

    connect(videoController_, &VideoController::streamingLost, this,
            &VideoWindow::onStreamingLost);

    connect(videoController_, &VideoController::startupTimeout, this,
            &VideoWindow::onStartupTimeout);

    // --- Controller → View / Window updates ---

    connect(videoController_, &VideoController::textStatsUpdated, imageViewer_,
            &ImageViewerWidget::setText);

    // ---  View → Controller for display stats ---

    connect(imageViewer_, &ImageViewerWidget::frameDisplayed, videoController_,
            &VideoController::onFrameDisplayed);

    // --- Application settings synchronization ---

    bindApplicationSettingsToController();
    bindApplicationSettingsToView();
    bindUiToApplicationSettings();

    const auto& app = ApplicationSettings::instance();

    // refresh widget in function of settings
    connect(&app, &ApplicationSettings::videoSettingsChanged, this,
            [this](const VideoSessionSettings& conf)
            {
                bool hasPreprocessing = conf.compute.downscale.downscaleEnabled ||
                                        conf.compute.spatialFilteringEnabled ||
                                        conf.compute.temporalFilteringEnabled;

                displayBar_->updateDisplayModeAvailability(hasPreprocessing);
            });

    // refresh button in function of user action
    connect(deviceSelector_, &QComboBox::currentIndexChanged, this,
            &VideoWindow::updateApplyButton);

    connect(videoController_, &VideoController::downscaleChanged, this,
            &VideoWindow::onDownscaleChanged);
}

void VideoWindow::applyInitialSettings()
{
    assert(imageViewer_ && displayBar_);

    const auto& app = ApplicationSettings::instance();
    const auto& downscaleParams = app.videoSettings().compute.downscale;

    imageViewer_->applyDownscaleConfig(downscaleParams);
    imageViewer_->applyDisplayConfig(app.videoSettings().display);

    bool preprocessing = app.videoSettings().compute.downscale.downscaleEnabled ||
                         app.videoSettings().compute.temporalFilteringEnabled;

    displayBar_->updateDisplayModeAvailability(preprocessing);

    loadLastSourceType();

    refreshSourceUi();
    refreshUi();

    onDownscaleChanged(downscaleParams);
}

void VideoWindow::bindApplicationSettingsToController()
{
    assert(videoController_);

    const auto& app = ApplicationSettings::instance();

    connect(&app, &ApplicationSettings::videoSettingsChanged, videoController_,
            &VideoController::onVideoSettingsChanged);

    connect(&app, &ApplicationSettings::videoDisplaySettingsChanged, videoController_,
            &VideoController::onVideoDisplaySettingsChanged);
}

void VideoWindow::bindApplicationSettingsToView()
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

void VideoWindow::bindUiToApplicationSettings()
{
    assert(displayBar_ && videoSettingsWindow_);

    const auto& app = ApplicationSettings::instance();

    connect(displayBar_, &DisplaySettingsWidget::displayConfigChanged, &app,
            &ApplicationSettings::setVideoDisplayConfig);

    // commit settings
    connect(videoSettingsWindow_, &VideoSettingsWindow::videoComputeSettingsAccepted, &app,
            &ApplicationSettings::setVideoComputeSettings);
}

void VideoWindow::refreshSourceUi()
{
    assert(sourceTypeCombo_ && sourceCombo_);

    bool cameraMode = sourceConfig_.type == SourceType::Camera;
    bool urlMode = sourceConfig_.type == SourceType::Url;
    bool fileMode = sourceConfig_.type == SourceType::File;

    deviceLabel_->setVisible(cameraMode);
    deviceSelector_->setVisible(cameraMode);
    formatLabel_->setVisible(cameraMode);
    formatSelector_->setVisible(cameraMode);

    openFileButton_->setVisible(fileMode);
    sourceCombo_->setVisible(!cameraMode);
    clearButton_->setVisible(!cameraMode);

    if (urlMode)
    {
        sourceCombo_->lineEdit()->setPlaceholderText(
            "https://video.mp4  https://stream.m3u8  rtsp://camera/live");
    }
    else if (fileMode)
    {
        sourceCombo_->lineEdit()->setPlaceholderText(tr("Open a local video file..."));
    }
}

void VideoWindow::updateSourceConfigFromUi()
{
    auto type = static_cast<SourceType>(sourceTypeCombo_->currentData().toInt());

    sourceConfig_.type = type;

    switch (sourceConfig_.type)
    {
        case SourceType::Camera:
            sourceConfig_.cameraId = deviceSelector_->currentData().toByteArray();

            sourceConfig_.cameraFormat = getSelectedFormat();
            return;

        case SourceType::Url:
            sourceConfig_.url = QUrl(sourceCombo_->currentText().trimmed());
            return;

        case SourceType::File:
            sourceConfig_.url = QUrl::fromUserInput(sourceCombo_->currentText().trimmed());
            return;
    }

    std::unreachable();
}

void VideoWindow::updateDeviceList(const QList<QCameraDevice>& devices)
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

        if (!lastKnownDeviceIds_.isEmpty() && !lastKnownDeviceIds_.contains(deviceId))
            newlyAddedCamera = deviceId;

        QIcon icon = deviceIdleIcon_;

        const auto state = deviceStreamingStatus_.value(deviceId, DeviceStreamingStatus::Idle);

        if (state == DeviceStreamingStatus::Streaming)
            icon = deviceActiveIcon_;
        else if (state == DeviceStreamingStatus::Error)
            icon = deviceErrorIcon_;

        deviceSelector_->addItem(icon, dev.description(), deviceId);
    }

    lastKnownDeviceIds_ = currentIds;

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

int VideoWindow::computeBestDeviceIndex(const QByteArray& previousSelection,
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

void VideoWindow::setDeviceControlsEnabled(bool enabled)
{
    assert(deviceSelector_ && toggleStreamingButton_);

    deviceSelector_->setEnabled(enabled);
    toggleStreamingButton_->setEnabled(enabled);
}

void VideoWindow::onToggleStreaming()
{
    assert(deviceSelector_ && videoController_);

    if (configChangeInProgress_)
        return;

    if (deviceSelector_->currentIndex() < 0)
        return;

    if (videoController_->isStreaming())
        stopSource();
    else
        startSource();
}

void VideoWindow::onApplySelection()
{
    assert(videoController_);

    if (configChangeInProgress_)
        return;

    if (!videoController_->isStreaming())
        return;

    configChangeInProgress_ = true;
    stopSource();
}

void VideoWindow::onDeviceChanged(int /*index*/)
{
    refreshFormatListFromSelection();
}

void VideoWindow::refreshFormatListFromSelection()
{
    assert(deviceSelector_ && formatSelector_ && videoController_);

    ScopedUiUpdateGuard guard(isUpdatingUi_);

    int index = deviceSelector_->currentIndex();
    if (index < 0)
    {
        formatSelector_->clear();
        return;
    }

    QByteArray deviceId = deviceSelector_->itemData(index).toByteArray();
    sourceConfig_.cameraId = deviceId;

    // 👉 récupérer directement depuis la liste actuelle
    const auto devices = videoController_->videoInputs();

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

void VideoWindow::updateFormatList(const QList<QCameraFormat>& formats)
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
    if (!sourceConfig_.cameraId.isEmpty())
    {
        auto preferred = preferredFormats_.value(sourceConfig_.cameraId);

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

bool VideoWindow::hasPendingConfiguration() const
{
    if (sourceConfig_.type != SourceType::Camera)
        return false;

    if (!videoController_->isStreaming())
        return false;

    if (sourceConfig_.cameraId.isEmpty() || streamingDeviceId_.isEmpty())
        return false;

    const auto selectedFormat = getSelectedFormat();

    if (selectedFormat.isNull() || activeFormat_.isNull())
        return false;

    return sourceConfig_.cameraId != streamingDeviceId_ ||
           !camera_utils::isSameCameraFormat(selectedFormat, activeFormat_);
}

void VideoWindow::showEvent(QShowEvent* event)
{
    emit videoWindowShown();
    QMainWindow::showEvent(event);
}

void VideoWindow::closeEvent(QCloseEvent* event)
{
    stopSource();

    saveSelectedCameraId();
    savePreferredFormats();

    QSettings settings;

    settings.setValue("ui_geometry/camera_window", saveGeometry());

    emit videoWindowClosed();
    QMainWindow::closeEvent(event);
}

#ifdef Q_OS_ANDROID
void VideoWindow::ensureCameraPermission()
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

void VideoWindow::connectFrameToView()
{
    disconnect(frameToViewConnection_);
    frameToViewConnection_ = connect(videoController_, &VideoController::imageAndContourUpdated,
                                     imageViewer_, &ImageViewerWidget::setImageAndContour);
}

QCameraFormat VideoWindow::getSelectedFormat() const
{
    assert(formatSelector_);

    int index = formatSelector_->currentIndex();

    if (index < 0)
        return QCameraFormat();

    return formatSelector_->itemData(index).value<QCameraFormat>();
}

void VideoWindow::startSource()
{
    assert(videoController_);

    updateSourceConfigFromUi();

    videoController_->start(sourceConfig_);
}

void VideoWindow::stopSource()
{
    assert(videoController_);

    videoController_->stop();
}

void VideoWindow::onStreamingStarting()
{
    refreshUi();
}

void VideoWindow::onStreamingStarted(const StreamingInfo& info)
{
    assert(imageViewer_);

    if (info.source.type == SourceType::Camera)
    {
        streamingDeviceId_ = info.source.deviceId;
        deviceStreamingStatus_[info.source.deviceId] = DeviceStreamingStatus::Streaming;
        preferredFormats_[info.source.deviceId] = info.source.deviceFormat;
        activeFormat_ = info.source.deviceFormat;
    }
    else if (info.source.sourceUrl.isLocalFile())
    {
        saveLastVideoDirectory(QFileInfo(info.source.sourceUrl.toLocalFile()).absolutePath());
    }

    if (!configChangeInProgress_)
    {
        imageViewer_->showPlaceholder(false);
        connectFrameToView();
    }

    refreshUi();

    sourceTitleStr_ = sourceTitle(info);

    updateWindowTitle();

    if (info.source.type == SourceType::Camera)
    {
        saveSelectedCameraId();
        savePreferredFormats();
    }
    else
    {
        addSourceToHistory(info.source.sourceUrl);
    }

    configChangeInProgress_ = false;
}

QString VideoWindow::lastVideoDirectory() const
{
    QSettings settings;

    return settings.value(kLastVideoDirectory).toString();
}

void VideoWindow::saveLastVideoDirectory(const QString& directory)
{
    if (directory.isEmpty())
        return;

    QSettings settings;

    settings.setValue(kLastVideoDirectory, directory);
}

QString VideoWindow::sourceTitle(const StreamingInfo& info) const
{
    QString title = info.source.description;

    if (info.frameSize.isValid())
    {
        title += QString(" - %1x%2").arg(info.frameSize.width()).arg(info.frameSize.height());
    }

    if (info.pixelFormat != QVideoFrameFormat::Format_Invalid)
    {
        title += QString(" %1").arg(pixelFormatToString(info.pixelFormat));
    }

    if (info.sourceFrameRate > 0.f)
    {
        title += QString(" @%1").arg(info.sourceFrameRate, 0, 'f', 0);
    }

    return title;
}

void VideoWindow::onStreamingStopped()
{
    assert(imageViewer_);

    if (!streamingDeviceId_.isEmpty())
    {
        if (deviceStreamingStatus_[streamingDeviceId_] == DeviceStreamingStatus::Streaming)
            deviceStreamingStatus_[streamingDeviceId_] = DeviceStreamingStatus::Idle;
    }

    streamingDeviceId_.clear();
    activeFormat_ = QCameraFormat();

    if (configChangeInProgress_)
    {
        startSource();
    }
    else
    {
        disconnect(frameToViewConnection_);
        imageViewer_->showPlaceholder(true);

        refreshUi();
        updateWindowTitle();
    }
}

void VideoWindow::onCameraError(const QByteArray& deviceId, QCamera::Error,
                                const QString& errorString)
{
    assert(imageViewer_);

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QMessageBox::warning(this, tr("Camera error"), errorString);

    deviceStreamingStatus_[deviceId] = DeviceStreamingStatus::Error;

    // un switch raté devient un stop
    if (configChangeInProgress_)
    {
        configChangeInProgress_ = false;
        streamingDeviceId_.clear();
    }

    refreshUi();
}

void VideoWindow::onStartupTimeout(const SourceInfo& sourceInfo, double timeoutSec)
{
    QMessageBox::warning(this, tr("Camera startup failed"),
                         tr("The camera did not produce a valid frame within %1 seconds.\n"
                            "The device may be busy or not responding.")
                             .arg(timeoutSec, 0, 'f', 1));

    if (sourceInfo.type == SourceType::Camera)
        deviceStreamingStatus_[sourceInfo.deviceId] = DeviceStreamingStatus::Error;

    refreshUi();
}

void VideoWindow::onStreamingLost(const StreamingInfo& streamingInfo, double frameAgeSec)
{
    assert(imageViewer_);

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QMessageBox::warning(
        this, tr("Camera stream lost"),
        tr("No valid frame received for %1 seconds.\nThe camera stream may have stalled.")
            .arg(frameAgeSec, 0, 'f', 1));

    if (streamingInfo.source.type == SourceType::Camera)
        deviceStreamingStatus_[streamingInfo.source.deviceId] = DeviceStreamingStatus::Error;

    refreshUi();
}

void VideoWindow::refreshActionButtons()
{
    updateStreamingButton();
    updateApplyButton();
}

void VideoWindow::updateStreamingButton()
{
    assert(videoController_ && toggleStreamingButton_ && deviceSelector_);

    switch (videoController_->streamingState())
    {
        case StreamingState::Stopped:
            toggleStreamingButton_->setEnabled(true);
            toggleStreamingButton_->setText(tr("Start"));
            toggleStreamingButton_->setToolTip(tr("Start camera streaming."));
            toggleStreamingButton_->setIcon(startIcon_);
            break;

        case StreamingState::Streaming:
            toggleStreamingButton_->setEnabled(true);
            toggleStreamingButton_->setText(tr("Stop"));
            toggleStreamingButton_->setToolTip(tr("Stop camera streaming."));
            toggleStreamingButton_->setIcon(stopIcon_);
            break;

        case StreamingState::Starting:
            toggleStreamingButton_->setEnabled(false);
            toggleStreamingButton_->setText(tr("Starting..."));
            toggleStreamingButton_->setToolTip(tr("Camera startup in progress."));
            toggleStreamingButton_->setIcon(QIcon());
            break;
    }
}

void VideoWindow::updateApplyButton()
{
    applyButton_->setVisible(hasPendingConfiguration());
}

void VideoWindow::refreshUi()
{
    assert(videoController_);

    updateDeviceList(videoController_->videoInputs());
    refreshActionButtons();
}

bool VideoWindow::isCameraAvailable() const
{
    assert(deviceSelector_);

    return deviceSelector_->count() > 0;
}

QString VideoWindow::pixelFormatToShortString(QVideoFrameFormat::PixelFormat fmt)
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
            return tr("Other");
    }
}

QString VideoWindow::formatToString(const QCameraFormat& fmt)
{
    const QSize res = fmt.resolution();
    const int fps = static_cast<int>(fmt.maxFrameRate());

    const QString pixelFormat = pixelFormatToShortString(fmt.pixelFormat());

    return QString("%1 %2x%3 @%4").arg(pixelFormat).arg(res.width()).arg(res.height()).arg(fps);
}

int VideoWindow::findBestFormatIndex(const QList<QCameraFormat>& formats) const
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

bool VideoWindow::isYuv420(QVideoFrameFormat::PixelFormat fmt) const
{
    return fmt == QVideoFrameFormat::Format_YUV420P || fmt == QVideoFrameFormat::Format_NV12 ||
           fmt == QVideoFrameFormat::Format_NV21;
}

bool VideoWindow::isYuv(QVideoFrameFormat::PixelFormat fmt) const
{
    return isYuv420(fmt) || fmt == QVideoFrameFormat::Format_YUYV;
}

bool VideoWindow::is640x480(const QSize& s) const
{
    return s.width() == 640 && s.height() == 480;
}

bool VideoWindow::is30fps(float fps) const
{
    return std::abs(fps - 30.0f) < 1.0f;
}

QByteArray VideoWindow::loadSelectedCameraId()
{
    QSettings settings;
    return settings.value(kCameraDeviceKey).toByteArray();
}

void VideoWindow::loadLastSourceType()
{
    QSettings settings;

    int value = settings.value(kLastSourceTypeKey, static_cast<int>(SourceType::Camera)).toInt();

    SourceType type = static_cast<SourceType>(value);

    int index = sourceTypeCombo_->findData(QVariant::fromValue(type));

    if (index >= 0)
    {
        QSignalBlocker blocker(sourceTypeCombo_);

        sourceTypeCombo_->setCurrentIndex(index);

        sourceConfig_.type = type;
    }
}

void VideoWindow::saveLastSourceType()
{
    QSettings settings;

    settings.setValue(kLastSourceTypeKey, static_cast<int>(sourceConfig_.type));
}

void VideoWindow::saveSelectedCameraId()
{
    QSettings settings;
    settings.setValue(kCameraDeviceKey, deviceSelector_->currentData().toByteArray());
}

void VideoWindow::savePreferredFormats()
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

void VideoWindow::loadPreferredFormats()
{
    assert(videoController_);

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

        const auto inputs = videoController_->videoInputs();

        for (const auto& cam : inputs)
        {
            QByteArray deviceId = device::decodeDeviceId(dev);

            if (cam.id() != deviceId)
                continue;

            const auto formats = cam.videoFormats();

            for (const auto& fmt : formats)
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

void VideoWindow::addSourceToHistory(const QUrl& url)
{
    QSignalBlocker blocker(sourceCombo_);

    QString value = url.toString();

    if (value.isEmpty())
        return;

    int existing = sourceCombo_->findText(value);

    if (existing >= 0)
        sourceCombo_->removeItem(existing);

    sourceCombo_->insertItem(0, value);

    sourceCombo_->setCurrentIndex(0);

    constexpr int kMaxHistory = 20;

    while (sourceCombo_->count() > kMaxHistory)
        sourceCombo_->removeItem(sourceCombo_->count() - 1);

    saveSourceHistory();
}

void VideoWindow::loadSourceHistory()
{
    QSettings settings;

    QStringList values = settings.value(kSourceHistoryKey).toStringList();

    sourceCombo_->addItems(values);
}

void VideoWindow::saveSourceHistory()
{
    QStringList values;

    for (int i = 0; i < sourceCombo_->count(); ++i)
        values << sourceCombo_->itemText(i);

    QSettings settings;
    settings.setValue(kSourceHistoryKey, values);
}

void VideoWindow::onDownscaleChanged(const DownscaleParams& downscaleParams)
{
    assert(videoController_);

    downscaleTitleStr_.clear();

    if (downscaleParams.downscaleEnabled)
        downscaleTitleStr_ = QString("(/%1)").arg(downscaleParams.downscaleFactor);

    updateWindowTitle();
}

void VideoWindow::updateWindowTitle()
{
    if (videoController_ && videoController_->isStreaming())
    {
        QString title = sourceTitleStr_;

        if (!downscaleTitleStr_.isEmpty())
            title += " " + downscaleTitleStr_;

        setWindowTitle(title);
    }
    else
    {
        setWindowTitle(tr("Video"));
    }
}

QString VideoWindow::pixelFormatToString(QVideoFrameFormat::PixelFormat format)
{
    using PF = QVideoFrameFormat::PixelFormat;

    switch (format)
    {
        case PF::Format_Invalid:
            return "Invalid";

        case PF::Format_ARGB8888:
            return "ARGB8888";

        case PF::Format_ARGB8888_Premultiplied:
            return "ARGB8888_Premultiplied";

        case PF::Format_XRGB8888:
            return "XRGB8888";

        case PF::Format_BGRA8888:
            return "BGRA8888";

        case PF::Format_BGRA8888_Premultiplied:
            return "BGRA8888_Premultiplied";

        case PF::Format_BGRX8888:
            return "BGRX8888";

        case PF::Format_ABGR8888:
            return "ABGR8888";

        case PF::Format_XBGR8888:
            return "XBGR8888";

        case PF::Format_RGBA8888:
            return "RGBA8888";

        case PF::Format_RGBX8888:
            return "RGBX8888";

        case PF::Format_AYUV:
            return "AYUV";

        case PF::Format_AYUV_Premultiplied:
            return "AYUV_Premultiplied";

        case PF::Format_YUV420P:
            return "YUV420P";

        case PF::Format_YUV422P:
            return "YUV422P";

        case PF::Format_YV12:
            return "YV12";

        case PF::Format_UYVY:
            return "UYVY";

        case PF::Format_YUYV:
            return "YUYV";

        case PF::Format_NV12:
            return "NV12";

        case PF::Format_NV21:
            return "NV21";

        case PF::Format_IMC1:
            return "IMC1";

        case PF::Format_IMC2:
            return "IMC2";

        case PF::Format_IMC3:
            return "IMC3";

        case PF::Format_IMC4:
            return "IMC4";

        case PF::Format_Y8:
            return "Y8";

        case PF::Format_Y16:
            return "Y16";

        case PF::Format_P010:
            return "P010";

        case PF::Format_P016:
            return "P016";

        case PF::Format_SamplerExternalOES:
            return "SamplerExternalOES";

        case PF::Format_Jpeg:
            return "JPEG";

        case PF::Format_SamplerRect:
            return "SamplerRect";

        case PF::Format_YUV420P10:
            return "YUV420P10";
    }

    return QString(tr("Unknown(%1)")).arg(static_cast<int>(format));
}

void VideoWindow::openFile()
{
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open Video File"), lastVideoDirectory(), file_utils::buildVideoFilter());

    if (filename.isEmpty())
        return;

    sourceTypeCombo_->setCurrentIndex(static_cast<int>(SourceType::File));

    sourceCombo_->setCurrentText(QUrl::fromLocalFile(filename).toString());

    if (videoController_->isStreaming())
    {
        configChangeInProgress_ = true;
        stopSource();
    }
    else
    {
        startSource();
    }
}

} // namespace fluvel
