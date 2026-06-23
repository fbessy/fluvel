// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include "video_types.hpp"

#include <QMainWindow>

#include <QByteArray>
#include <QCamera>
#include <QElapsedTimer>
#include <QIcon>
#include <QMediaPlayer>
#include <QMetaObject>
#include <QSet>
#include <QString>
#include <QUrl>

class QWidget;
class QComboBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QStringListModel;
class QCompleter;
class QSlider;
class QStackedLayout;

class QShowEvent;
class QCloseEvent;

namespace fluvel
{

struct StreamingInfo;
class VideoController;
class VideoSettingsDialog;
class RightPanelToggleButton;
class DisplaySettingsWidget;
class ImageViewerWidget;
class TimelineSlider;
class VolumeSlider;

/**
 * @brief Streaming status for a camera device.
 */
enum class DeviceStreamingStatus
{
    Idle,
    Streaming,
    Error
};

/**
 * @brief Stores a preferred camera format.
 *
 * A format is considered valid if it has a valid resolution
 * and pixel format.
 */
struct SavedFormat
{
    QSize resolution;
    float fps{0.f};
    QVideoFrameFormat::PixelFormat pixelFormat = QVideoFrameFormat::Format_Invalid;

    bool isValid() const
    {
        return resolution.isValid() && pixelFormat != QVideoFrameFormat::Format_Invalid;
    }
};

/**
 * @brief RAII guard to temporarily disable UI updates.
 *
 * Sets the given flag to true on construction and restores it
 * to false on destruction.
 */
class ScopedUiUpdateGuard
{
public:
    explicit ScopedUiUpdateGuard(bool& flag)
        : flag_(flag)
    {
        flag_ = true;
    }

    ~ScopedUiUpdateGuard()
    {
        flag_ = false;
    }

private:
    bool& flag_;
};

/**
 * @brief Stores information about the last reported error.
 *
 * This structure is used to suppress duplicate error notifications
 * occurring within a short time interval.
 *
 * Two errors are considered duplicates when they originate from the
 * same source and report the same message within the configured
 * deduplication window.
 */
struct LastReportedError
{
    /**
     * @brief Source associated with the error.
     */
    QString source;

    /**
     * @brief Error message used for duplicate detection.
     */
    QString message;

    /**
     * @brief Time elapsed since the error was last reported.
     */
    QElapsedTimer timer;
};

/**
 * @brief Main window for video source selection, streaming and visualization.
 *
 * This window provides the user interface to:
 * - select a video source (camera, URL or file)
 * - configure camera formats when applicable
 * - start and stop video streaming
 * - display frames in an ImageViewerWidget
 * - configure processing and display settings
 *
 * It coordinates interactions between:
 * - VideoController (video acquisition and streaming)
 * - ImageViewerWidget (display)
 * - settings widgets
 *
 * The class also manages source history, device availability,
 * preferred camera formats and streaming state.
 */
class VideoWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the video window.
     */
    explicit VideoWindow(QWidget* parent = nullptr);

signals:

    /**
     * @brief Emitted when the window is shown.
     */
    void videoWindowShown();

    /**
     * @brief Emitted when the window is closed.
     */
    void videoWindowClosed();

protected:
    /**
     * @brief Handles window show events.
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Handles window close events.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    void onDownscaleChanged(const DownscaleParams& downscaleParams);
    void updateWindowTitle();

    void setupWindow();
    void restoreSettings();
    void createUi();
    static QIcon createActiveFormatIcon();
    void setupView();
    void setupController();
    void setupLayout();
    void applyInitialSettings();
    void setupConnections();
    void bindApplicationSettingsToController();
    void bindApplicationSettingsToView();
    void bindUiToApplicationSettings();
    void connectFrameToView();

    void refreshSourceUi();
    void updateSourceConfigFromUi(int sourceTypeComboIndex);

    void updateDeviceList(const QList<QCameraDevice>& devices);
    int computeBestDeviceIndex(const QByteArray& previousSelection, const QByteArray& newlyPlugged);
    void setDeviceControlsEnabled(bool enabled);
    bool canStartSource() const;
    void updateActionBar();
    void updateStreamingButton();
    void updateApplyButton();
    void refreshUi();

    void openFile();
    void openMediaFile(const QString& filename);

    void onToggleStreaming();
    void onApplySelection();

    void onDeviceChanged(int index);
    void refreshFormatListFromSelection();
    void updateFormatList(const QList<QCameraFormat>& formats);
    bool hasPendingConfiguration() const;
    QCameraFormat getSelectedFormat() const;

    void loadPreferredFormats();
    void savePreferredFormats();

    void addSourceToHistory(const QUrl& url);
    void loadSourceHistory();
    void saveSourceHistory();

    QString lastVideoDirectory() const;
    void saveLastVideoDirectory(const QString& directory);

    void onStreamingStarting();
    void onStreamingStarted(const StreamingInfo& info);
    void onStreamingStopped();

    void onCameraError(const CameraErrorInfo& errorInfo);
    void onMediaPlayerError(const MediaPlayerErrorInfo& errorInfo);

    bool shouldReportMediaError(const MediaPlayerErrorInfo& errorInfo);

    void onStartupTimeout(const SourceInfo& sourceInfo, double timeoutSec);
    void onStreamingLost(const StreamingInfo& streamingInfo, double frameAgeSec);

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    void startSource();
    void stopSource();

    void loadLastSourceType();
    void saveLastSourceType();

    static QByteArray loadSelectedCameraId();
    void saveSelectedCameraId();

    QString sourceTitle(const StreamingInfo& info) const;

    void updateSourceCompleter();
    void onSourceContextMenuRequested(const QPoint& pos);

    void onMediaInfoChanged(const MediaInfo& info);
    QString buildSourceTitle() const;
    void appendStreamingInfo(QString& title, const StreamingInfo& info) const;

    void updateMediaControls();

    void onPlaybackPositionChanged(qint64 pos);
    void setSeekControlsVisible(bool visible);

    void updatePlayPauseButton(bool paused);
    void togglePause();

    void onVolumeChanged(int value);
    void updateVolumeIcon(int volume);
    void toggleMute();
    void saveVolume();

    void toggleFullscreen();
    void enterFullscreen();
    void leaveFullscreen();

    QStringListModel* sourceCompleterModel_{nullptr};
    QCompleter* sourceCompleter_{nullptr};

    VideoSettingsDialog* videoSettingsWindow_{nullptr};

    QWidget* central_{nullptr};

    QLabel* sourceTypeLabel_{nullptr};
    QComboBox* sourceTypeCombo_{nullptr};
    QWidget* sourceTypeWidget_{nullptr};

    QLabel* deviceLabel_{nullptr};
    QComboBox* deviceSelector_{nullptr};
    QWidget* deviceWidget_{nullptr};

    QLabel* formatLabel_{nullptr};
    QComboBox* formatSelector_{nullptr};
    QWidget* formatWidget_{nullptr};

    QWidget* cameraConfigWidget_{nullptr};

    QPushButton* openFileButton_{nullptr};
    QComboBox* urlCombo_{nullptr};
    QIcon clearHistoryIcon_;
    QWidget* fileUrlConfigWidget_{nullptr};

    QWidget* sourceConfigWidget_{nullptr};
    QStackedLayout* sourceConfigStack_{nullptr};

    RightPanelToggleButton* rightPanelToggle_{nullptr};

    QIcon settingsIcon_;
    QPushButton* settingsButton_{nullptr};

    QWidget* configRightBlockWidget_{nullptr};

    QIcon startIcon_;
    QIcon stopIcon_;
    QPushButton* toggleStreamingButton_{nullptr};

    QPushButton* applyButton_{nullptr};

    DisplaySettingsWidget* displayBar_{nullptr};

    QSet<QByteArray> lastKnownDeviceIds_;

    ImageViewerWidget* imageViewer_{nullptr};
    VideoController* videoController_{nullptr};
    QMetaObject::Connection frameToViewConnection_;

    QHash<QByteArray, DeviceStreamingStatus> deviceStreamingStatus_;

    QIcon deviceActiveIcon_;
    QIcon deviceIdleIcon_;
    QIcon deviceErrorIcon_;

    QIcon formatActiveIcon_;
    QIcon formatAvailableIcon_;

    bool configChangeInProgress_{false};

    QHash<QByteArray, QCameraFormat> preferredFormats_;

    bool isUpdatingUi_{false};

    QString downscaleTitleStr_;
    QString sourceTitleStr_;

    SourceConfig sourceConfig_{};

    LastReportedError lastReportedError_{};

    QWidget* mediaControlsWidget_{nullptr};

    QPushButton* playPauseButton_{nullptr};
    QIcon resumeIcon_;
    QIcon pauseIcon_;

    TimelineSlider* playbackSlider_{nullptr};

    QLabel* playbackPositionLabel_{nullptr};
    QLabel* playbackDurationLabel_{nullptr};

    StreamingInfo streamingInfo_{};
    MediaInfo mediaInfo_{};

    QPushButton* volumeButton_{nullptr};
    VolumeSlider* volumeSlider_{nullptr};
    QLabel* volumeLabel_{nullptr};
    QMenu* volumeMenu_{nullptr};

    QIcon volumeMuteIcon_;
    QIcon volumeLowIcon_;
    QIcon volumeMediumIcon_;
    QIcon volumeHighIcon_;

    QWidget* controlBar_{nullptr};

    int lastNonZeroVolume_{50};

    bool isFullScreen_{false};
};

} // namespace fluvel
