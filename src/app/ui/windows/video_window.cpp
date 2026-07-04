// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_window.hpp"
#include "animated_push_button.hpp"
#include "application_settings.hpp"
#include "autofit_behavior.hpp"
#include "camera_format_utils.hpp"
#include "clickable_label.hpp"
#include "device_id_utils.hpp"
#include "display_settings_widget.hpp"
#include "drag_drop_behavior.hpp"
#include "file_utils.hpp"
#include "fullscreen_behavior.hpp"
#include "fullscreen_video_control_bar.hpp"
#include "icon_loader.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "qcolor_utils.hpp"
#include "qt_utils.hpp"
#include "right_panel_toggle_button.hpp"
#include "time_utils.hpp"
#include "timeline_slider.hpp"
#include "video_controller.hpp"
#include "video_format_utils.hpp"
#include "video_settings_dialog.hpp"
#include "video_types.hpp"
#include "volume_controller.hpp"
#include "volume_slider.hpp"

#include <QAbstractItemView>
#include <QCameraDevice>
#include <QComboBox>
#include <QCompleter>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QStackedLayout>
#include <QStringListModel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <utility>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QPermission>
#include <QtCore/qpermissions.h>
#endif

namespace
{
constexpr auto kLastSourceTypeKey = "sources/last_type";
constexpr auto kCameraDeviceKey = "camera/device";
constexpr auto kCameraFormatsKey = "camera/formats";
constexpr auto kSourceHistoryKey = "sources/history";
constexpr auto kLastVideoDirectory = "video/last_directory";

constexpr auto kVolumeKey = "media/volume";
constexpr auto kMutedKey = "media/muted";

constexpr int kDefaultVolume = 50;
constexpr bool kDefaultMuted = false;
constexpr int kSaveAudioSettingsDelayMs = 3000;

constexpr int kControlSpacing = 6;
constexpr int kSectionSpacing = 12;
constexpr int kGroupSpacing = 20;

constexpr int kVerticalSpacing = 4;
} // namespace

namespace fluvel
{

VideoWindow::VideoWindow(QWidget* parent)
    : QMainWindow(parent)
    , shortcutManager_(this)

{
    saveAudioSettingsTimer_.setSingleShot(true);

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

    sourceTypeLabel_ = new QLabel(tr("Source: "));

    QIcon cameraIcon =
        il::loadIcon(QIcon::ThemeIcon::CameraVideo, ":/icons/actions/camera-video-symbolic.svg");

    QIcon videoIcon = il::loadIcon("video-x-generic-symbolic", ":/icons/file/video-symbolic.svg");

    sourceTypeCombo_ = new QComboBox(this);
    sourceTypeCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    sourceTypeCombo_->addItem(cameraIcon, tr("Camera"), QVariant::fromValue(SourceType::Camera));
    sourceTypeCombo_->addItem(videoIcon, tr("File / URL"), QVariant::fromValue(SourceType::Media));
    sourceTypeCombo_->setToolTip(tr("Select a camera, video file, or network stream."));

    auto* sourceTypeLayout = new QHBoxLayout;
    sourceTypeLayout->setContentsMargins(0, 0, 0, 0);
    sourceTypeLayout->addWidget(sourceTypeLabel_);
    sourceTypeLayout->addWidget(sourceTypeCombo_);
    sourceTypeWidget_ = new QWidget;
    sourceTypeWidget_->setLayout(sourceTypeLayout);

    deviceLabel_ = new QLabel(tr("Device: "));

    deviceSelector_ = new QComboBox(this);
    deviceSelector_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    deviceSelector_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    deviceSelector_->setMinimumContentsLength(15);

    static constexpr int kCameraIconSize{13};
    deviceSelector_->setIconSize(QSize(kCameraIconSize, kCameraIconSize));

    auto* deviceLayout = new QHBoxLayout;
    deviceLayout->setContentsMargins(0, 0, 0, 0);
    deviceLayout->addWidget(deviceLabel_);
    deviceLayout->addWidget(deviceSelector_);
    deviceWidget_ = new QWidget;
    deviceWidget_->setLayout(deviceLayout);

    QColor greenActive("#4FC98A");
    QColor orangeError("#FF9F0A");

    deviceActiveIcon_ = il::createDisk(greenActive);
    deviceIdleIcon_ = il::createEmpty(kCameraIconSize);
    deviceErrorIcon_ = il::createDisk(orangeError);

    formatLabel_ = new QLabel(tr("Format: "));

    formatSelector_ = new QComboBox(this);
    formatSelector_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formatSelector_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    formatSelector_->setMinimumContentsLength(15);

    static constexpr int kFormatIconSize{16};
    formatSelector_->setIconSize(QSize(kFormatIconSize, kFormatIconSize));
    formatSelector_->setToolTip(tr("Camera resolution, frame rate and pixel format."));

    auto* formatLayout = new QHBoxLayout;
    formatLayout->setContentsMargins(0, 0, 0, 0);
    formatLayout->addWidget(formatLabel_);
    formatLayout->addWidget(formatSelector_);
    formatWidget_ = new QWidget;
    formatWidget_->setLayout(formatLayout);

    auto* cameraConfigLayout = new QHBoxLayout;
    cameraConfigLayout->setContentsMargins(0, 0, 0, 0);
    cameraConfigLayout->setSpacing(kSectionSpacing);
    cameraConfigLayout->addWidget(deviceWidget_);
    cameraConfigLayout->addWidget(formatWidget_);
    cameraConfigLayout->addStretch();
    cameraConfigWidget_ = new QWidget;
    cameraConfigWidget_->setLayout(cameraConfigLayout);

    formatActiveIcon_ = createActiveFormatIcon();
    formatAvailableIcon_ = il::createEmpty(kFormatIconSize);

    openFileButton_ = new AnimatedPushButton(tr("Open..."));
    openFileButton_->setIcon(videoIcon);
    openFileButton_->setToolTip(tr("Select a local video file."));

    QIcon networkIcon = il::loadIcon(":/icons/actions/globe-symbolic.svg");

    urlCombo_ = new QComboBox(this);

    urlCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    urlCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    urlCombo_->setMinimumContentsLength(30);

    urlCombo_->setEditable(true);

    urlCombo_->lineEdit()->addAction(networkIcon, QLineEdit::LeadingPosition);

    urlCombo_->lineEdit()->setPlaceholderText(
        "https://video.mp4  https://stream.m3u8  rtsp://camera/live");

    urlCombo_->setToolTip(tr("Media URL examples:\n\n"
                             "HTTP video:\n"
                             "https://host/video.mp4\n\n"
                             "HLS stream:\n"
                             "https://host/stream.m3u8\n\n"
                             "RTSP stream:\n"
                             "rtsp://host/camera/live\n\n"
                             "Local network camera:\n"
                             "https://192.168.1.110:8080/video\n"
                             "rtsp://192.168.1.110:1935/live"));

    urlCombo_->lineEdit()->setClearButtonEnabled(true);

    urlCombo_->lineEdit()->setContextMenuPolicy(Qt::CustomContextMenu);

    clearHistoryIcon_ =
        il::loadIcon(QIcon::ThemeIcon::EditClear, ":/icons/actions/edit-clear-history.svg");

    connect(urlCombo_->lineEdit(), &QWidget::customContextMenuRequested, this,
            &VideoWindow::onSourceContextMenuRequested);

    sourceCompleterModel_ = new QStringListModel(this);

    sourceCompleter_ = new QCompleter(sourceCompleterModel_, urlCombo_);
    sourceCompleter_->setCaseSensitivity(Qt::CaseInsensitive);
    sourceCompleter_->setCompletionMode(QCompleter::PopupCompletion);

    urlCombo_->setCompleter(sourceCompleter_);

    auto* fileUrlConfigLayout = new QHBoxLayout;
    fileUrlConfigLayout->setContentsMargins(0, 0, 0, 0);
    fileUrlConfigLayout->setSpacing(kSectionSpacing);

    fileUrlConfigLayout->addWidget(openFileButton_);
    fileUrlConfigLayout->addWidget(urlCombo_, 1);
    fileUrlConfigWidget_ = new QWidget;
    fileUrlConfigWidget_->setLayout(fileUrlConfigLayout);

    sourceConfigStack_ = new QStackedLayout;
    sourceConfigStack_->setContentsMargins(0, 0, 0, 0);

    sourceConfigStack_->addWidget(cameraConfigWidget_);
    sourceConfigStack_->addWidget(fileUrlConfigWidget_);

    sourceConfigWidget_ = new QWidget;
    sourceConfigWidget_->setLayout(sourceConfigStack_);

    rightPanelToggle_ = new RightPanelToggleButton;

    settingsButton_ = new AnimatedPushButton;
    settingsButton_->setToolTip(tr("Open video session settings."));
    settingsButton_->setFlat(true);
    settingsButton_->setFocusPolicy(Qt::NoFocus);

    settingsIcon_ = il::loadIcon("configure", ":/icons/actions/settings-symbolic.svg");

    settingsButton_->setIcon(settingsIcon_);

    auto* configRightBlockLayout = new QHBoxLayout;
    configRightBlockLayout->setContentsMargins(0, 0, 0, 0);
    configRightBlockLayout->addWidget(rightPanelToggle_);
    configRightBlockLayout->addWidget(settingsButton_);
    configRightBlockWidget_ = new QWidget;
    configRightBlockWidget_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    configRightBlockWidget_->setLayout(configRightBlockLayout);

    startIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                              ":/icons/media/media-playback-start-symbolic.svg");

    stopIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                             ":/icons/media/media-playback-stop-symbolic.svg");

    startIconLight_ =
        il::loadIcon(":/icons/media/media-playback-start-symbolic.svg", il::IconMode::Light);

    stopIconLight_ =
        il::loadIcon(":/icons/media/media-playback-stop-symbolic.svg", il::IconMode::Light);

    toggleStreamingButton_ = new AnimatedPushButton;
    toggleStreamingButton_->setTransitionEffect(TransitionEffect::Slide);
    toggleStreamingButton_->setClickAnimation(ClickAnimation::None);

    QIcon applyIcon = createActiveFormatIcon();

    applyButton_ = new QPushButton;
    applyButton_->setIcon(applyIcon);
    applyButton_->setEnabled(false);
    applyButton_->setFlat(true);
    applyButton_->setText(tr("Apply"));
    applyButton_->setToolTip(tr("Restart the active source using the selected configuration."));

    // --- Display bar ---
    displayBar_ = new DisplaySettingsWidget(config.display, central_);

    videoSettingsWindow_ = new VideoSettingsDialog(config.compute, this);

    // --- Play/Pause button ---

    resumeIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                               ":/icons/media/media-playback-start-symbolic.svg");

    pauseIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackPause,
                              ":/icons/media/media-playback-pause-symbolic.svg");

    resumeIconLight_ =
        il::loadIcon(":/icons/media/media-playback-start-symbolic.svg", il::IconMode::Light);

    pauseIconLight_ =
        il::loadIcon(":/icons/media/media-playback-pause-symbolic.svg", il::IconMode::Light);

    playPauseButton_ = new AnimatedPushButton;
    playPauseButton_->setTransitionEffect(TransitionEffect::Flip);
    playPauseButton_->setClickAnimation(ClickAnimation::None);
    playPauseButton_->setIcon(resumeIcon_);

    // --- Playback widgets ---
    playbackSlider_ = new TimelineSlider(this, ui::Appearance::Native);
    playbackSlider_->setMinimumWidth(250);

    const QString positionPlaceholder("0:00:00");
    const QString durationPlaceholder("-0:00:00");

    playbackPositionLabel_ = new QLabel(positionPlaceholder);
    playbackDurationLabel_ = new ClickableLabel(durationPlaceholder);

    QFontMetrics fm(font());

    // Reserve enough space to avoid layout shifts when the
    // displayed time format changes (e.g. when crossing one hour).
    const int positionTextWidth = fm.horizontalAdvance(positionPlaceholder);
    playbackPositionLabel_->setMinimumWidth(positionTextWidth);

    const int durationTextWidth = fm.horizontalAdvance(durationPlaceholder);
    playbackDurationLabel_->setMinimumWidth(durationTextWidth);

    playbackPositionLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    playbackDurationLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    playbackSeparatorLabel_ = new QLabel("/");

    // --- Volume widgets ---
    volumeMuteIcon_ =
        il::loadIcon(QIcon::ThemeIcon::AudioVolumeMuted, ":/icons/status/audio-volume-muted.svg");

    volumeLowIcon_ =
        il::loadIcon(QIcon::ThemeIcon::AudioVolumeLow, ":/icons/status/audio-volume-low.svg");

    volumeMediumIcon_ =
        il::loadIcon(QIcon::ThemeIcon::AudioVolumeMedium, ":/icons/status/audio-volume-medium.svg");

    volumeHighIcon_ =
        il::loadIcon(QIcon::ThemeIcon::AudioVolumeHigh, ":/icons/status/audio-volume-high.svg");

    volumeButton_ = new QPushButton(this);
    volumeButton_->setContextMenuPolicy(Qt::CustomContextMenu);

    volumeSlider_ = new VolumeSlider(this, ui::Appearance::Native);
    volumeSlider_->setRange(0, 100);
    volumeSlider_->setFixedWidth(150);

    volumeLabel_ = new QLabel("100%");
    volumeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    volumeLabel_->setFixedWidth(volumeLabel_->fontMetrics().horizontalAdvance("100%"));

    volumeMenu_ = new QMenu(this);

    auto* popupWidget = new QWidget;

    auto* popupLayout = new QHBoxLayout(popupWidget);
    popupLayout->addWidget(volumeSlider_);
    popupLayout->addWidget(volumeLabel_);
    popupLayout->setContentsMargins(8, 8, 8, 8);

    auto* action = new QWidgetAction(volumeMenu_);
    action->setDefaultWidget(popupWidget);

    volumeMenu_->addAction(action);

    mediaControlsWidget_ = new QWidget(this);

    auto* mediaLayout = new QHBoxLayout(mediaControlsWidget_);
    mediaLayout->setContentsMargins(0, 0, 0, 0);
    mediaLayout->setSpacing(kControlSpacing);

    mediaLayout->addWidget(playPauseButton_);
    mediaLayout->addSpacing(kControlSpacing);
    mediaLayout->addWidget(volumeButton_);

    mediaLayout->addSpacing(2 * kControlSpacing);

    mediaLayout->addWidget(playbackPositionLabel_);
    mediaLayout->addWidget(playbackSeparatorLabel_);
    mediaLayout->addWidget(playbackDurationLabel_);
    mediaLayout->addWidget(playbackSlider_, 1);
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

    fullscreenBar_ = new FullscreenVideoControlBar(imageViewer_);
    fullscreenBar_->hide();

    fullscreenBar_->startStopButton()->setIcon(startIconLight_);
    fullscreenBar_->playPauseButton()->setIcon(resumeIconLight_);
    fullscreenBar_->volumeController()->setControlsEnabled(false);

    fullscreenOpacity_ = new QGraphicsOpacityEffect(fullscreenBar_);
    fullscreenOpacity_->setOpacity(0.0);
    fullscreenBar_->setGraphicsEffect(fullscreenOpacity_);

    showAnimation_ = new QPropertyAnimation(fullscreenOpacity_, "opacity", this);

    showAnimation_->setDuration(100);
    showAnimation_->setStartValue(0.0);
    showAnimation_->setEndValue(1.0);

    hideAnimation_ = new QPropertyAnimation(fullscreenOpacity_, "opacity", this);

    hideAnimation_->setDuration(250);
    hideAnimation_->setStartValue(1.0);
    hideAnimation_->setEndValue(0.0);
}

void VideoWindow::setupController()
{
    const auto& config = ApplicationSettings::instance().videoSettings();
    videoController_ = new VideoController(config, this);
}

void VideoWindow::setupLayout()
{
    auto* vLayout = new QVBoxLayout(central_);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    //
    // Top control bar
    //
    controlBar_ = new QWidget(central_);

    //
    // Source column
    //
    auto* sourceTypeColumn = new QVBoxLayout;
    sourceTypeColumn->setContentsMargins(0, 0, 0, 0);
    sourceTypeColumn->setSpacing(kVerticalSpacing);

    sourceTypeColumn->addWidget(sourceTypeWidget_);
    sourceTypeColumn->addWidget(toggleStreamingButton_);

    //
    // Apply column
    //
    auto* applyColumn = new QVBoxLayout;
    applyColumn->setContentsMargins(0, 0, 0, 0);
    applyColumn->setSpacing(kVerticalSpacing);

    applyColumn->addStretch(); // vide au dessus
    applyColumn->addWidget(applyButton_);

    //
    // Left column
    //
    auto* leftColumn = new QHBoxLayout;
    leftColumn->setContentsMargins(0, 0, 0, 0);
    leftColumn->setSpacing(kSectionSpacing);

    leftColumn->addLayout(sourceTypeColumn);
    leftColumn->addLayout(applyColumn);

    //
    // Config column
    //
    auto* configColumn = new QVBoxLayout;
    configColumn->setContentsMargins(0, 0, 0, 0);
    configColumn->setSpacing(kVerticalSpacing);

    configColumn->addWidget(sourceConfigWidget_);
    configColumn->addStretch();
    configColumn->addWidget(mediaControlsWidget_);

    //
    // Right column
    //
    auto* rightColumn = new QVBoxLayout;
    rightColumn->setContentsMargins(0, 0, 0, 0);
    rightColumn->setSpacing(kVerticalSpacing);

    rightColumn->addWidget(configRightBlockWidget_);
    rightColumn->addStretch(); // vide dessous

    //
    // Main top bar layout
    //

    auto* topBarLayout = new QHBoxLayout(controlBar_);
    topBarLayout->setContentsMargins(8, 4, 8, 4);
    topBarLayout->setSpacing(kGroupSpacing);

    topBarLayout->addLayout(leftColumn);
    topBarLayout->addLayout(configColumn);
    topBarLayout->addLayout(rightColumn);

    //
    // Video area
    //
    auto* contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(imageViewer_, 1);
    contentLayout->addWidget(displayBar_, 0);

    //
    // Main layout
    //
    vLayout->addWidget(controlBar_);
    vLayout->addLayout(contentLayout, 1);

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

                refreshUi();
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

    connect(urlCombo_->lineEdit(), &QLineEdit::textChanged, this,
            [this]()
            {
                updateSourceConfigFromUi(sourceTypeCombo_->currentIndex());
                updateActionBar();
            });

    connect(openFileButton_, &QPushButton::clicked, this, &VideoWindow::openFile);

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

    connect(videoController_, &VideoController::playbackPositionChanged, this,
            &VideoWindow::onPlaybackPositionChanged);

    connect(playbackDurationLabel_, &ClickableLabel::clicked, this,
            &VideoWindow::toggleTimeDisplayMode);

    connect(fullscreenBar_->durationLabel(), &ClickableLabel::clicked, this,
            &VideoWindow::toggleTimeDisplayMode);

    connect(playbackSlider_, &QSlider::sliderReleased, this,
            [this]
            {
                videoController_->seek(playbackSlider_->value());
            });

    connect(playbackSlider_, &QSlider::sliderMoved, this,
            [this]
            {
                playbackPositionLabel_->setText(
                    time_utils::formatDuration(playbackSlider_->value()));

                updateDurationLabel();

                videoController_->seek(playbackSlider_->value());
            });

    connect(videoController_, &VideoController::mediaInfoChanged, this,
            &VideoWindow::onMediaInfoChanged);

    connect(volumeSlider_, &QSlider::valueChanged, this, &VideoWindow::volumeRequested);

    connect(volumeButton_, &QPushButton::clicked, this,
            [this]()
            {
                volumeMenu_->popup(volumeButton_->mapToGlobal(QPoint(0, volumeButton_->height())));
            });

    connect(volumeButton_, &QWidget::customContextMenuRequested, this,
            [this]()
            {
                toggleMute();
            });

    connect(playPauseButton_, &QPushButton::clicked, this, &VideoWindow::togglePause);

    connect(videoController_, &VideoController::pausedChanged, this,
            &VideoWindow::updatePlayPauseButton);

    connect(imageViewer_, &ImageViewerWidget::toggleFullscreenRequested, this,
            &VideoWindow::toggleFullscreen);

    // fullscreen bar connections

    connect(imageViewer_, &ImageViewerWidget::activityDetected, this,
            &VideoWindow::onActivityDetected);

    connect(imageViewer_, &ImageViewerWidget::idle, this, &VideoWindow::onIdle);

    connect(hideAnimation_, &QPropertyAnimation::finished, this,
            [this]()
            {
                if (fullscreenOpacity_->opacity() < 0.01)
                {
                    fullscreenBar_->hide();
                }
            });

    connect(fullscreenBar_->startStopButton(), &QPushButton::clicked, this,
            &VideoWindow::onToggleStreaming);

    connect(fullscreenBar_->playPauseButton(), &QPushButton::clicked, this,
            &VideoWindow::togglePause);

    connect(fullscreenBar_->playbackSlider(), &QSlider::sliderReleased, this,
            [this]
            {
                videoController_->seek(fullscreenBar_->playbackSlider()->value());
            });

    connect(fullscreenBar_->playbackSlider(), &QSlider::sliderMoved, this,
            [this]
            {
                const auto value = fullscreenBar_->playbackSlider()->value();

                fullscreenBar_->positionLabel()->setText(time_utils::formatDuration(value));

                videoController_->seek(value);
            });

    connect(fullscreenBar_->cameraSelector(), &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                deviceSelector_->setCurrentIndex(index);

                if (hasPendingConfiguration())
                    onApplySelection();
            });

    connect(deviceSelector_, &QComboBox::currentIndexChanged, fullscreenBar_->cameraSelector(),
            &QComboBox::setCurrentIndex);

    connect(fullscreenBar_->mirrorButton(), &QToolButton::toggled, displayBar_,
            &DisplaySettingsWidget::setMirrorModeEnabled);

    connect(fullscreenBar_->smoothButton(), &QToolButton::toggled, displayBar_,
            &DisplaySettingsWidget::setSmoothDisplayEnabled);

    connect(fullscreenBar_->overlayButton(), &QToolButton::toggled, displayBar_,
            &DisplaySettingsWidget::setAlgorithmOverlayEnabled);

    connect(fullscreenBar_->volumeController(), &VolumeController::volumeRequested, this,
            &VideoWindow::volumeRequested);

    connect(fullscreenBar_->volumeController(), &VolumeController::toggleMuteRequested, this,
            &VideoWindow::toggleMute);

    connect(&shortcutManager_, &VideoShortcutManager::playPauseRequested, this,
            &VideoWindow::togglePause);

    connect(&shortcutManager_, &VideoShortcutManager::toggleMuteRequested, this,
            &VideoWindow::toggleMute);

    connect(&shortcutManager_, &VideoShortcutManager::toggleFullscreenRequested, this,
            &VideoWindow::toggleFullscreen);

    connect(&shortcutManager_, &VideoShortcutManager::escapeRequested, this,
            &VideoWindow::leaveFullscreen);

    connect(&shortcutManager_, &VideoShortcutManager::seekRequested, this,
            &VideoWindow::stepPlayback);

    connect(&shortcutManager_, &VideoShortcutManager::volumeRequested, this,
            &VideoWindow::stepVolume);

    connect(videoController_, &VideoController::volumeChanged, this, &VideoWindow::onVolumeChanged);

    connect(videoController_, &VideoController::mutedChanged, this, &VideoWindow::onMutedChanged);

    connect(&saveAudioSettingsTimer_, &QTimer::timeout, this, &VideoWindow::saveAudioSettings);
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

    applyInitialAudioSettings();

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
    assert(sourceTypeCombo_ && urlCombo_);

    const bool cameraMode = (sourceConfig_.type == SourceType::Camera);

    sourceConfigStack_->setCurrentWidget(cameraMode ? cameraConfigWidget_ : fileUrlConfigWidget_);
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
            sourceConfig_.url = QUrl::fromUserInput(urlCombo_->currentText().trimmed());
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

    qt_utils::copyComboBox(deviceSelector_, fullscreenBar_->cameraSelector());

    if (currentIndex >= 0)
        onDeviceChanged(currentIndex);
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

    if (restartPending_)
        return;

    if (videoController_->isStreaming())
        stopSource();
    else
        startSource();
}

void VideoWindow::onApplySelection()
{
    assert(videoController_);

    if (restartPending_)
        return;

    if (!videoController_->isStreaming())
        return;

    restartPending_ = true;
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
    saveAudioSettings();

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

    if (!canStartSource())
        return;

    streamingInfo_ = {};
    mediaInfo_ = {};

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

    streamingInfo_ = info;

    if (info.source.type == SourceType::Camera)
    {
        deviceStreamingStatus_[info.source.deviceId] = DeviceStreamingStatus::Streaming;
        preferredFormats_[info.source.deviceId] = info.source.deviceFormat;
    }
    else if (info.source.sourceUrl.isLocalFile())
    {
        saveLastVideoDirectory(QFileInfo(info.source.sourceUrl.toLocalFile()).absolutePath());
    }

    if (!restartPending_)
    {
        imageViewer_->showPlaceholder(false);
        connectFrameToView();
    }

    refreshUi();

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

    restartPending_ = false;
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

    if (title.isEmpty() && info.source.type == SourceType::Media)
    {
        const QUrl& url = info.source.sourceUrl;

        if (url.isLocalFile())
        {
            title = url.fileName();
        }
        else
        {
            title = url.host();

            if (title.isEmpty())
                title = url.fileName();
        }
    }

    if (title.isEmpty())
        title = tr("Video");

    appendStreamingInfo(title, info);

    return title;
}

void VideoWindow::onStreamingStopped()
{
    assert(imageViewer_);

    streamingInfo_ = {};
    mediaInfo_ = {};
    updateDurationLabel();

    volumeButton_->setEnabled(false);
    fullscreenBar_->volumeController()->setControlsEnabled(false);

    for (auto& state : deviceStreamingStatus_)
    {
        if (state == DeviceStreamingStatus::Streaming)
            state = DeviceStreamingStatus::Idle;
    }

    if (restartPending_)
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

void VideoWindow::onCameraError(const CameraErrorInfo& errorInfo)
{
    assert(imageViewer_);

    restartPending_ = false;

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QString message =
        tr("Source: %1\n\n%2").arg(errorInfo.sourceInfo.description, errorInfo.errorString);

    QMessageBox::warning(this, tr("Camera error"), message);

    deviceStreamingStatus_[errorInfo.sourceInfo.deviceId] = DeviceStreamingStatus::Error;

    refreshUi();
}

void VideoWindow::onMediaPlayerError(const MediaPlayerErrorInfo& errorInfo)
{
    assert(imageViewer_ && fullscreenBar_);

    if (!shouldShowMediaError(errorInfo))
        return;

    restartPending_ = false;

    disconnect(frameToViewConnection_);
    imageViewer_->showPlaceholder(true);

    QString message =
        tr("Source: %1\n\n%2").arg(errorInfo.sourceInfo.description, errorInfo.errorString);

    QMessageBox::warning(this, tr("Media error"), message);

    refreshUi();

    volumeButton_->setEnabled(false);
    fullscreenBar_->volumeController()->setControlsEnabled(false);
}

bool VideoWindow::shouldShowMediaError(const MediaPlayerErrorInfo& errorInfo)
{
    if (errorInfo.state != StreamingState::Streaming)
        return true;

    constexpr int kDeduplicationDelayMs = 5000;

    const bool sameSource = lastReportedError_.source == errorInfo.sourceInfo.description;

    const bool sameMessage = lastReportedError_.message == errorInfo.errorString;

    const bool recent = lastReportedError_.timer.isValid() &&
                        lastReportedError_.timer.elapsed() < kDeduplicationDelayMs;

    if (sameSource && sameMessage && recent)
        return false;

    lastReportedError_.source = errorInfo.sourceInfo.description;
    lastReportedError_.message = errorInfo.errorString;
    lastReportedError_.timer.restart();

    return true;
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
            const QString text = urlCombo_->currentText().trimmed();

            if (text.isEmpty())
                return false;

            const QUrl url = QUrl::fromUserInput(text);

            if (!url.isValid())
                return false;

            if (url.isLocalFile())
                return file_utils::isSupportedVideoFile(url.toLocalFile());

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

    const bool canStart = canStartSource();

    switch (videoController_->streamingState())
    {
        case StreamingState::Stopped:
            toggleStreamingButton_->setEnabled(canStart);
            toggleStreamingButton_->setText(tr("Start"));
            toggleStreamingButton_->setToolTip(tr("Start selected source."));
            toggleStreamingButton_->setAnimatedIcon(startIcon_);

            fullscreenBar_->startStopButton()->setEnabled(canStart);
            fullscreenBar_->startStopButton()->setAnimatedIcon(startIconLight_,
                                                               TransitionDirection::Left);
            break;

        case StreamingState::Streaming:
            toggleStreamingButton_->setEnabled(true);
            toggleStreamingButton_->setText(tr("Stop"));
            toggleStreamingButton_->setToolTip(tr("Stop active source."));
            toggleStreamingButton_->setAnimatedIcon(stopIcon_);

            fullscreenBar_->startStopButton()->setAnimatedIcon(stopIconLight_,
                                                               TransitionDirection::Right);
            break;

        case StreamingState::Starting:
            toggleStreamingButton_->setEnabled(false);
            toggleStreamingButton_->setText(tr("Starting..."));
            toggleStreamingButton_->setToolTip(tr("Camera startup in progress."));
            // toggleStreamingButton_->setIcon(QIcon());
            break;
    }
}

void VideoWindow::updateApplyButton()
{
    applyButton_->setEnabled(hasPendingConfiguration());
}

void VideoWindow::refreshUi()
{
    assert(videoController_);

    updateDeviceList(videoController_->videoInputs());
    updateActionBar();
    updateMediaBar();
    updateFullscreenBar();
}

QByteArray VideoWindow::loadSelectedCameraId()
{
    QSettings settings;
    return settings.value(kCameraDeviceKey).toByteArray();
}

void VideoWindow::applyInitialAudioSettings()
{
    QSettings settings;
    int volume = settings.value(kVolumeKey, kDefaultVolume).toInt();
    volume = std::clamp(volume, 0, 100);

    const bool muted = settings.value(kMutedKey, kDefaultMuted).toBool();

    // Reuse the runtime UI update handlers to initialize the audio widgets.
    // Signal connections are established afterwards, so no feedback loop occurs.
    onVolumeChanged(static_cast<float>(volume) / 100.f);
    onMutedChanged(muted);

    volumeRequested(volume);
    videoController_->setMuted(muted);
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
    QSignalBlocker blocker(urlCombo_);

    QString value = url.toString();

    if (value.isEmpty())
        return;

    int existing = urlCombo_->findText(value);

    if (existing >= 0)
        urlCombo_->removeItem(existing);

    urlCombo_->insertItem(0, value);

    urlCombo_->setCurrentIndex(0);

    constexpr int kMaxHistory = 20;

    while (urlCombo_->count() > kMaxHistory)
        urlCombo_->removeItem(urlCombo_->count() - 1);

    saveSourceHistory();

    updateSourceCompleter();
}

void VideoWindow::loadSourceHistory()
{
    QSettings settings;

    QStringList values = settings.value(kSourceHistoryKey).toStringList();

    urlCombo_->addItems(values);

    updateSourceCompleter();
}

void VideoWindow::saveSourceHistory()
{
    QStringList values;

    for (int i = 0; i < urlCombo_->count(); ++i)
        values << urlCombo_->itemText(i);

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
    if (!videoController_ || !videoController_->isStreaming())
    {
        setWindowTitle(tr("Video"));
        return;
    }

    sourceTitleStr_ = buildSourceTitle();

    if (!downscaleTitleStr_.isEmpty())
        sourceTitleStr_ += " " + downscaleTitleStr_;

    setWindowTitle(sourceTitleStr_);
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

    if (!file_utils::isSupportedVideoFile(filename))
        return;

    int index = sourceTypeCombo_->findData(QVariant::fromValue(SourceType::Media));
    sourceTypeCombo_->setCurrentIndex(index);

    urlCombo_->setCurrentText(QUrl::fromLocalFile(filename).toString());

    if (videoController_->isStreaming())
    {
        restartPending_ = true;
        stopSource();
    }
    else
    {
        startSource();
    }
}

void VideoWindow::updateSourceCompleter()
{
    QStringList entries;

    entries << "https://";
    entries << "rtsp://";
    entries << "file://";

    for (int i = 0; i < urlCombo_->count(); ++i)
        entries << urlCombo_->itemText(i);

    entries.removeDuplicates();

    sourceCompleterModel_->setStringList(entries);
}

void VideoWindow::onSourceContextMenuRequested(const QPoint& pos)
{
    QLineEdit* edit = urlCombo_->lineEdit();

    QMenu* menu = edit->createStandardContextMenu();

    menu->addSeparator();

    QAction* clearHistoryAction = menu->addAction(clearHistoryIcon_, tr("Clear source history"));

    const bool hasHistory = urlCombo_->count() > 0;
    clearHistoryAction->setEnabled(hasHistory);

    QAction* selected = menu->exec(edit->mapToGlobal(pos));

    if (selected == clearHistoryAction)
    {
        urlCombo_->clear();
        saveSourceHistory();
        updateSourceCompleter();
    }

    delete menu;
}

void VideoWindow::toggleTimeDisplayMode()
{
    if (timeDisplayMode_ == TimeDisplayMode::TotalDuration)
        timeDisplayMode_ = TimeDisplayMode::RemainingTime;
    else
        timeDisplayMode_ = TimeDisplayMode::TotalDuration;

    updateDurationLabel();
}

void VideoWindow::updateDurationLabel()
{
    QString text;

    if (timeDisplayMode_ == TimeDisplayMode::TotalDuration)
    {
        text = time_utils::formatDuration(mediaInfo_.durationMs);
    }
    else
    {
        const int pos = playbackSlider_->value();
        const qint64 remaining = std::max<qint64>(0, mediaInfo_.durationMs - pos);

        text = "-" + time_utils::formatDuration(remaining);
    }

    playbackDurationLabel_->setText(text);
    fullscreenBar_->durationLabel()->setText(text);
}

void VideoWindow::onPlaybackPositionChanged(qint64 pos)
{
    assert(playbackSlider_ && playbackPositionLabel_ && playbackDurationLabel_ && fullscreenBar_);

    if (playbackSlider_->isSliderDown() || fullscreenBar_->playbackSlider()->isSliderDown())
        return;

    const int posInt = static_cast<int>(pos);

    playbackSlider_->setValue(posInt);
    fullscreenBar_->playbackSlider()->setValue(posInt);

    const auto position = time_utils::formatDuration(pos);

    playbackPositionLabel_->setText(position);
    fullscreenBar_->positionLabel()->setText(position);

    updateDurationLabel();
}

void VideoWindow::onMediaInfoChanged(const MediaInfo& info)
{
    assert(playbackSlider_ && playbackDurationLabel_ && videoController_ && playPauseButton_ &&
           volumeButton_ && fullscreenBar_);

    mediaInfo_ = info;

    const int durationInt = static_cast<int>(mediaInfo_.durationMs);

    playbackSlider_->setRange(0, durationInt);
    fullscreenBar_->playbackSlider()->setRange(0, durationInt);

    updateDurationLabel();

    updatePlayPauseButton(videoController_->isPaused());

    refreshUi();

    updateWindowTitle();
}

QString VideoWindow::buildSourceTitle() const
{
    QString title;

    if (!mediaInfo_.title.isEmpty())
    {
        title = mediaInfo_.title;
        appendStreamingInfo(title, streamingInfo_);
    }
    else
    {
        title = sourceTitle(streamingInfo_);
    }

    return title;
}

void VideoWindow::appendStreamingInfo(QString& title, const StreamingInfo& info) const
{
    if (info.frameSize.isValid())
    {
        title += QString(" - %1x%2").arg(info.frameSize.width()).arg(info.frameSize.height());
    }

    if (info.pixelFormat != QVideoFrameFormat::Format_Invalid)
    {
        title += QString(" %1").arg(video_utils::pixelFormatToString(info.pixelFormat));
    }

    if (info.sourceFrameRate > 0.0)
    {
        title += QString(" @%1").arg(info.sourceFrameRate, 0, 'f', 0);
    }
    else if (mediaInfo_.frameRate > 0.0)
    {
        title += QString(" @%1").arg(mediaInfo_.frameRate, 0, 'f', 0);
    }
}

void VideoWindow::volumeRequested(int value)
{
    value = std::clamp(value, 0, 100);

    videoController_->setVolume(static_cast<float>(value) / 100.f);
}

void VideoWindow::toggleMute()
{
    if (!mediaInfo_.hasAudio)
        return;

    videoController_->setMuted(!videoController_->isMuted());
}

void VideoWindow::saveAudioSettings()
{
    QSettings settings;

    settings.setValue(kVolumeKey, qRound(videoController_->volume() * 100.f));

    settings.setValue(kMutedKey, videoController_->isMuted());
}

void VideoWindow::updateVolumeIcon(int volume, bool muted)
{
    volume = std::clamp(volume, 0, 100);

    if (muted || volume == 0)
    {
        volumeButton_->setIcon(volumeMuteIcon_);
    }
    else if (volume < 33)
    {
        volumeButton_->setIcon(volumeLowIcon_);
    }
    else if (volume < 66)
    {
        volumeButton_->setIcon(volumeMediumIcon_);
    }
    else
    {
        volumeButton_->setIcon(volumeHighIcon_);
    }

    volumeButton_->setToolTip(tr("Volume: %1%").arg(volume));
}

void VideoWindow::updatePlayPauseButton(bool paused)
{
    playPauseButton_->setAnimatedIcon(paused ? resumeIcon_ : pauseIcon_);

    fullscreenBar_->playPauseButton()->setAnimatedIcon(paused ? resumeIconLight_ : pauseIconLight_);
}

void VideoWindow::togglePause()
{
    if (!videoController_->isMediaActive())
        return;

    if (videoController_->isPaused())
    {
        videoController_->resume();
    }
    else
    {
        videoController_->pause();
    }
}

void VideoWindow::toggleFullscreen()
{
    if (!isFullScreen_)
    {
        enterFullscreen();
    }
    else
    {
        leaveFullscreen();
    }
}

void VideoWindow::enterFullscreen()
{
    imageViewer_->enterFullscreenMode();

    controlBar_->hide();
    displayBar_->hide();

    showFullScreen();

    positionFullscreenBar();

    isFullScreen_ = true;
}

void VideoWindow::leaveFullscreen()
{
    fullscreenBar_->hide();
    imageViewer_->leaveFullscreenMode();

    showNormal();

    controlBar_->show();
    displayBar_->show();

    isFullScreen_ = false;
}

void VideoWindow::positionFullscreenBar()
{
    constexpr int kBottomMargin = 16;

    fullscreenBar_->adjustSize();

    const QSize size = fullscreenBar_->sizeHint();

    fullscreenBar_->resize(size);

    const int x = (imageViewer_->width() - size.width()) / 2;
    const int y = imageViewer_->height() - size.height() - kBottomMargin;

    fullscreenBar_->move(x, y);
}

void VideoWindow::onActivityDetected(const QPoint& pos)
{
    if (!isFullScreen_)
        return;

    constexpr int kTriggerZoneHeight = 150;

    const bool inBottomZone = pos.y() > imageViewer_->height() - kTriggerZoneHeight;

    if (!inBottomZone)
        return;

    positionFullscreenBar();

    if (!fullscreenBar_->isVisible())
    {
        fullscreenBar_->show();

        showAnimation_->stop();
        hideAnimation_->stop();

        showAnimation_->start();
    }
}

void VideoWindow::onIdle()
{
    if (!isFullScreen_)
        return;

    if (fullscreenBar_->underMouse())
        return;

    if (fullscreenBar_->cameraSelector()->view()->isVisible())
        return;

    showAnimation_->stop();
    hideAnimation_->stop();

    hideAnimation_->start();
}

void VideoWindow::updateMediaBar()
{
    const bool mediaMode =
        streamingInfo_.source.type == SourceType::Media ||
        (streamingInfo_.source.type == SourceType::None && sourceConfig_.type == SourceType::Media);

    const bool hasPlayPause = videoController_->isMediaActive();
    const bool hasSeek = mediaMode && mediaInfo_.seekable;
    const bool hasAudio = mediaMode && mediaInfo_.hasAudio;

    // Media playback controls are only available for media sources.
    playPauseButton_->setVisible(mediaMode);
    playbackPositionLabel_->setVisible(mediaMode);
    playbackSeparatorLabel_->setVisible(mediaMode);
    playbackDurationLabel_->setVisible(mediaMode);
    playbackSlider_->setVisible(mediaMode);
    volumeButton_->setVisible(mediaMode);

    // Seeking controls remain visible but are disabled when seeking
    // is not supported by the current media.
    playPauseButton_->setEnabled(hasPlayPause);
    playbackPositionLabel_->setEnabled(hasSeek);
    playbackSeparatorLabel_->setEnabled(hasSeek);
    playbackDurationLabel_->setEnabled(hasSeek);
    playbackSlider_->setEnabled(hasSeek);

    // Volume controls remain visible for media sources but are disabled
    // when the current media has no audio track.
    volumeButton_->setEnabled(hasAudio);
}

void VideoWindow::updateFullscreenBar()
{
    const bool mediaMode =
        streamingInfo_.source.type == SourceType::Media ||
        (streamingInfo_.source.type == SourceType::None && sourceConfig_.type == SourceType::Media);

    const bool cameraMode = streamingInfo_.source.type == SourceType::Camera ||
                            (streamingInfo_.source.type == SourceType::None &&
                             sourceConfig_.type == SourceType::Camera);

    const bool hasPlayPause = videoController_->isMediaActive();
    const bool hasSeek = mediaMode && mediaInfo_.seekable;
    const bool hasAudio = mediaMode && mediaInfo_.hasAudio;

    // Camera controls are only relevant for live camera sources.
    fullscreenBar_->cameraSelector()->setVisible(cameraMode);

    // Media playback controls are only available for media sources.
    fullscreenBar_->playPauseButton()->setVisible(mediaMode);
    fullscreenBar_->positionLabel()->setVisible(mediaMode);
    fullscreenBar_->durationLabel()->setVisible(mediaMode);
    fullscreenBar_->playbackSlider()->setVisible(mediaMode);
    fullscreenBar_->volumeController()->setControlsVisible(mediaMode);

    // Seeking controls remain visible but are disabled when seeking
    // is not supported by the current media.
    fullscreenBar_->playPauseButton()->setEnabled(hasPlayPause);
    fullscreenBar_->positionLabel()->setEnabled(hasSeek);
    fullscreenBar_->durationLabel()->setEnabled(hasSeek);
    fullscreenBar_->playbackSlider()->setEnabled(hasSeek);

    // Volume controls remain visible for media sources but are disabled
    // when the current media has no audio track.
    fullscreenBar_->volumeController()->setControlsEnabled(hasAudio);
}

void VideoWindow::stepPlayback(qint64 deltaMs)
{
    if (!videoController_->isMediaActive() || !mediaInfo_.seekable)
        return;

    videoController_->seek(videoController_->positionMs() + deltaMs);
}

void VideoWindow::stepVolume(int delta)
{
    if (!mediaInfo_.hasAudio)
        return;

    int value = qRound(videoController_->volume() * 100.f);

    value = std::clamp(value + delta, 0, 100);

    videoController_->setVolume(static_cast<float>(value) / 100.f);
}

void VideoWindow::onVolumeChanged(float volume)
{
    const int value = qRound(volume * 100.f);

    QSignalBlocker b1(volumeSlider_);
    QSignalBlocker b2(fullscreenBar_->volumeController());

    volumeSlider_->setValue(value);
    fullscreenBar_->volumeController()->setVolume(value);

    volumeLabel_->setText(QString("%1%").arg(value));

    updateVolumeIcon(value, videoController_->isMuted());

    saveAudioSettingsTimer_.start(kSaveAudioSettingsDelayMs);
}

void VideoWindow::onMutedChanged(bool muted)
{
    fullscreenBar_->volumeController()->setMuted(muted);

    const int volume = qRound(videoController_->volume() * 100.f);

    updateVolumeIcon(volume, muted);

    saveAudioSettingsTimer_.start(kSaveAudioSettingsDelayMs);
}

} // namespace fluvel
