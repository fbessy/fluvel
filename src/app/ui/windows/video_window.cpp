// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_window.hpp"
#include "application_settings.hpp"
#include "autofit_behavior.hpp"
#include "camera_format_utils.hpp"
#include "device_id_utils.hpp"
#include "display_settings_widget.hpp"
#include "drag_drop_behavior.hpp"
#include "file_utils.hpp"
#include "fullscreen_behavior.hpp"
#include "icon_loader.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "qcolor_utils.hpp"
#include "right_panel_toggle_button.hpp"
#include "video_controller.hpp"
#include "video_format_utils.hpp"
#include "video_settings_dialog.hpp"
#include "video_types.hpp"

#include <QCameraDevice>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
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
    sourceTypeCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    sourceTypeCombo_->addItem(tr("Camera"), QVariant::fromValue(SourceType::Camera));
    sourceTypeCombo_->addItem(tr("File / URL"), QVariant::fromValue(SourceType::Media));
    sourceTypeCombo_->setToolTip(tr("Select a camera, video file, or network stream."));

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
    formatSelector_->setToolTip(tr("Camera resolution, frame rate and pixel format."));

    formatActiveIcon_ = createActiveFormatIcon();
    formatAvailableIcon_ = createEmptyIcon(kFormatIconSize);

    openFileButton_ = new QPushButton(tr("Open..."));
    openFileButton_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    openFileButton_->setToolTip(tr("Select a local video file."));

    sourceCombo_ = new QComboBox(this);
    sourceCombo_->setEditable(true);
    sourceCombo_->lineEdit()->setPlaceholderText(
        "https://video.mp4  https://stream.m3u8  rtsp://camera/live  "
        "https://192.168.1.110:8080/video");

    clearButton_ = new QPushButton(tr("Clear"));
    QIcon deleteIcon =
        il::loadIcon(QIcon::ThemeIcon::EditClear, ":/icons/actions/edit-clear-history.svg");

    clearButton_->setIcon(deleteIcon);
    clearButton_->setToolTip(tr("Remove all saved source addresses."));

    startIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                              ":/icons/media/media-playback-start-symbolic.svg");

    stopIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                             ":/icons/media/media-playback-stop-symbolic.svg");

    toggleStreamingButton_ = new QPushButton;

    applyButton_ = new QPushButton;
    applyButton_->setVisible(false);
    applyButton_->setFlat(true);
    applyButton_->setText(tr("Apply"));
    applyButton_->setToolTip(tr("Restart the active source using the selected configuration."));

    QIcon applyIcon = createActiveFormatIcon();
    applyButton_->setIcon(applyIcon);

    rightPanelToggle_ = new RightPanelToggleButton;

    settingsButton_ = new QPushButton;
    settingsButton_->setToolTip(tr("Open video session settings."));
    settingsButton_->setFlat(true);
    settingsButton_->setFocusPolicy(Qt::NoFocus);

    settingsIcon_ = il::loadIcon("configure", ":/icons/actions/settings-symbolic.svg");

    settingsButton_->setIcon(settingsIcon_);

    // --- Display bar ---
    displayBar_ = new DisplaySettingsWidget(config.display, central_);

    videoSettingsWindow_ = new VideoSettingsDialog(config.compute, this);
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
    interaction->addBehavior(std::make_unique<DragDropBehavior>(
        DragDropContent::Videos, tr("Drop a video here\n\nor click Open...")));
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

    QVBoxLayout* controlBarLayout = new QVBoxLayout(controlBar);
    controlBarLayout->setContentsMargins(8, 4, 8, 4);
    controlBarLayout->setSpacing(8);

    QHBoxLayout* configLayout = new QHBoxLayout;
    configLayout->setSpacing(4);

    QHBoxLayout* actionLayout = new QHBoxLayout;

    configLayout->addWidget(sourceLabel_);
    configLayout->addWidget(sourceTypeCombo_);
    configLayout->addSpacing(20);

    configLayout->addWidget(deviceLabel_);
    configLayout->addWidget(deviceSelector_);

    configLayout->addWidget(formatLabel_);
    configLayout->addWidget(formatSelector_);

    configLayout->addWidget(openFileButton_);

    configLayout->addWidget(sourceCombo_, 1);
    configLayout->addWidget(clearButton_);

    configLayout->addStretch();

    configLayout->addWidget(rightPanelToggle_);
    configLayout->addSpacing(8);
    configLayout->addWidget(settingsButton_);

    actionLayout->addWidget(toggleStreamingButton_);
    actionLayout->addWidget(applyButton_);
    actionLayout->addStretch();

    int sourceWidth = sourceLabel_->sizeHint().width() + sourceTypeCombo_->sizeHint().width() +
                      configLayout->spacing();

    toggleStreamingButton_->setFixedWidth(sourceWidth);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(imageViewer_, 1);
    contentLayout->addWidget(displayBar_, 0);

    controlBarLayout->addLayout(configLayout);
    controlBarLayout->addLayout(actionLayout);

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
                updateSourceConfigFromUi(index);

                saveLastSourceType();

                refreshSourceUi();
                updateActionBar();
            });

    connect(deviceSelector_, &QComboBox::currentIndexChanged, this, &VideoWindow::onDeviceChanged);

    connect(formatSelector_, &QComboBox::currentIndexChanged, this,
            [this]()
            {
                if (isUpdatingUi_)
                    return;

                auto fmt = getSelectedFormat();

                if (!fmt.isNull() && !sourceConfig_.cameraId.isEmpty())
                {
                    sourceConfig_.cameraFormat = fmt;
                    savePreferredFormats();
                }

                updateApplyButton();
            });

    connect(sourceCombo_->lineEdit(), &QLineEdit::textChanged, this,
            [this]()
            {
                updateSourceConfigFromUi(sourceTypeCombo_->currentIndex());
                updateActionBar();
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
            &VideoSettingsDialog::show);

    // when the user drag and drop a video in the view of the video window.
    connect(imageViewer_, &ImageViewerWidget::imageDropped, this, &VideoWindow::openMediaFile);

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
    connect(videoController_, &VideoController::mediaPlayerError, this,
            &VideoWindow::onMediaPlayerError);

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
    connect(videoSettingsWindow_, &VideoSettingsDialog::videoComputeSettingsAccepted, &app,
            &ApplicationSettings::setVideoComputeSettings);
}

void VideoWindow::refreshSourceUi()
{
    assert(sourceTypeCombo_ && sourceCombo_);

    bool cameraMode = (sourceConfig_.type == SourceType::Camera);
    bool mediaMode = (sourceConfig_.type == SourceType::Media);

    deviceLabel_->setVisible(cameraMode);
    deviceSelector_->setVisible(cameraMode);
    formatLabel_->setVisible(cameraMode);
    formatSelector_->setVisible(cameraMode);

    openFileButton_->setVisible(mediaMode);
    sourceCombo_->setVisible(mediaMode);
    clearButton_->setVisible(mediaMode);
}

void VideoWindow::updateSourceConfigFromUi(int sourceTypeComboIndex)
{
    sourceConfig_ = {};

    sourceConfig_.type = sourceTypeCombo_->itemData(sourceTypeComboIndex).value<SourceType>();

    switch (sourceConfig_.type)
    {
        case SourceType::Camera:
            sourceConfig_.cameraId = deviceSelector_->currentData().toByteArray();
            sourceConfig_.cameraFormat = getSelectedFormat();
            return;

        case SourceType::Media:
            sourceConfig_.url = QUrl::fromUserInput(sourceCombo_->currentText().trimmed());
            return;

        case SourceType::None:
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

    if (!hasDevice)
        formatSelector_->clear();

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
    assert(deviceSelector_ && videoController_);

    const auto streamingDeviceId = videoController_->activeSource().deviceId;

    int index = -1;

    // 1 newly plugged camera
    if (index < 0 && !newlyPlugged.isEmpty())
        index = deviceSelector_->findData(newlyPlugged);

    // 2 user's previous selection
    if (index < 0 && !previousSelection.isEmpty())
        index = deviceSelector_->findData(previousSelection);

    // 3 active camera
    if (index < 0 && !streamingDeviceId.isEmpty())
        index = deviceSelector_->findData(streamingDeviceId);

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
    formatSelector_->setEnabled(enabled);
}

void VideoWindow::onToggleStreaming()
{
    assert(deviceSelector_ && videoController_);

    if (configChangeInProgress_)
        return;

    if (videoController_->isStreaming())
        stopSource();
    else if (canStartSource())
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
    assert(formatSelector_ && videoController_);

    ScopedUiUpdateGuard guard(isUpdatingUi_);
    QSignalBlocker blocker(formatSelector_);

    formatSelector_->clear();

    int bestFormatIndex = camera_utils::findBestFormatIndex(formats);

    int indexToSelect = -1;
    const auto activeFormat = videoController_->activeSource().deviceFormat;

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& fmt = formats[i];

        bool isActive = camera_utils::isSameCameraFormat(fmt, activeFormat);
        bool isRecommended = (i == bestFormatIndex);

        QString label = camera_utils::formatToString(fmt);

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
    if (!videoController_->isStreaming())
        return false;

    if (!canStartSource())
        return false;

    return !videoController_->activeSource().matches(sourceConfig_);
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
    assert(videoController_ && sourceTypeCombo_);

    updateSourceConfigFromUi(sourceTypeCombo_->currentIndex());

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
        deviceStreamingStatus_[info.source.deviceId] = DeviceStreamingStatus::Streaming;
        preferredFormats_[info.source.deviceId] = info.source.deviceFormat;
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
        title += QString(" %1").arg(video_utils::pixelFormatToString(info.pixelFormat));
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

    for (auto& state : deviceStreamingStatus_)
    {
        if (state == DeviceStreamingStatus::Streaming)
            state = DeviceStreamingStatus::Idle;
    }

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

void VideoWindow::onCameraError(const SourceInfo& sourceInfo, QCamera::Error,
                                const QString& errorString)
{
    assert(imageViewer_);

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QString message = tr("Source: %1\n\n%2").arg(sourceInfo.description).arg(errorString);

    QMessageBox::warning(this, tr("Camera error"), message);

    deviceStreamingStatus_[sourceInfo.deviceId] = DeviceStreamingStatus::Error;

    // un switch raté devient un stop
    if (configChangeInProgress_)
    {
        configChangeInProgress_ = false;
    }

    refreshUi();
}

void VideoWindow::onMediaPlayerError(const SourceInfo& sourceInfo, QMediaPlayer::Error,
                                     const QString& errorString)
{
    assert(imageViewer_);

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QString message = tr("Source: %1\n\n%2").arg(sourceInfo.description).arg(errorString);

    QMessageBox::warning(this, tr("Media error"), message);

    if (configChangeInProgress_)
    {
        configChangeInProgress_ = false;
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

void VideoWindow::updateActionBar()
{
    updateStreamingButton();
    updateApplyButton();
}

bool VideoWindow::canStartSource() const
{
    switch (sourceConfig_.type)
    {
        case SourceType::Camera:
            return deviceSelector_->count() > 0 && deviceSelector_->currentIndex() >= 0;

        case SourceType::Media:
        {
            QString text = sourceCombo_->currentText().trimmed();

            if (text.isEmpty())
                return false;

            QUrl url = QUrl::fromUserInput(text);

            if (!url.isValid())
                return false;

            if (url.isLocalFile())
            {
                return QFileInfo(url.toLocalFile()).exists();
            }

            return true;
        }

        case SourceType::None:
            return false;
    }

    return false;
}

void VideoWindow::updateStreamingButton()
{
    assert(videoController_ && toggleStreamingButton_ && deviceSelector_);

    switch (videoController_->streamingState())
    {
        case StreamingState::Stopped:
            toggleStreamingButton_->setEnabled(canStartSource());
            toggleStreamingButton_->setText(tr("Start"));
            toggleStreamingButton_->setToolTip(tr("Start selected source."));
            toggleStreamingButton_->setIcon(startIcon_);
            break;

        case StreamingState::Streaming:
            toggleStreamingButton_->setEnabled(true);
            toggleStreamingButton_->setText(tr("Stop"));
            toggleStreamingButton_->setToolTip(tr("Stop active source."));
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
    updateActionBar();
}

bool VideoWindow::isCameraAvailable() const
{
    assert(deviceSelector_);

    return deviceSelector_->count() > 0;
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

    SourceType type = SourceType::Camera;

    switch (static_cast<SourceType>(value))
    {
        case SourceType::None:
        case SourceType::Camera:
        case SourceType::Media:
            type = static_cast<SourceType>(value);
            break;
    }

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

void VideoWindow::openFile()
{
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open Video File"), lastVideoDirectory(), file_utils::buildVideoFilter());

    openMediaFile(filename);
}

void VideoWindow::openMediaFile(const QString& filename)
{
    if (filename.isEmpty())
        return;

    QFileInfo fi(filename);

    if (!fi.exists())
        return;

    int index = sourceTypeCombo_->findData(QVariant::fromValue(SourceType::Media));
    sourceTypeCombo_->setCurrentIndex(index);

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
