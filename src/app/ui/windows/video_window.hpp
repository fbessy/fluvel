// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include "video_shortcut_manager.hpp"
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
#include <QTimer>
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

class QGraphicsOpacityEffect;
class QPropertyAnimation;

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
class FullscreenVideoControlBar;
class ClickableLabel;
class AnimatedPushButton;

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
    /**
     * @brief Defines how the media duration label is displayed.
     */
    enum class TimeDisplayMode
    {
        TotalDuration, ///< Show the total media duration.
        RemainingTime  ///< Show the remaining playback time.
    };

    void onDownscaleChanged(const DownscaleParams& downscaleParams);

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

    void refreshSourceUi();
    void updateSourceConfigFromUi(int sourceTypeComboIndex);

    void updateDeviceList(const QList<QCameraDevice>& devices);
    int computeBestDeviceIndex(const QByteArray& previousSelection, const QByteArray& newlyPlugged);
    void setDeviceControlsEnabled(bool enabled);
    bool canStartSource() const;
    void updateActionBar();
    void updateStreamingButton();
    void updateApplyButton();
    void updateRecordingButton();
    void refreshUi();

    void openFile();
    void openMediaFile(const QString& filename);

    void onToggleStreaming();
    void onToggleRecording();
    void onApplySelection();

    void onWindowDeviceChanged(int index);
    void onFullscreenDeviceChanged(int index);

    void refreshFormatListFromSelection();
    void updateFormatList(const QList<QCameraFormat>& formats);
    bool hasPendingConfiguration() const;
    QCameraFormat getSelectedFormat() const;

    void connectFrameToView();

    void startSource();
    void stopSource();

    void onStreamingStarting();
    void onStreamingStarted(const StreamingInfo& info);
    void onStreamingStopped();

    void onRecordingStateChanged(RecorderState state);
    void onRecordingStatsChanged(const RecorderStats& stats);
    void onRecordingStarted(const QString& filename);
    void onRecordingFinalized(const QString& filename);

    //
    // Error handling
    //

    // Backend errors
    void onCameraError(const CameraErrorInfo& errorInfo);
    void onMediaPlayerError(const MediaPlayerErrorInfo& errorInfo);

    bool shouldShowMediaError(const MediaPlayerErrorInfo& errorInfo);

    // Application-level errors
    void onStartupTimeout(const SourceInfo& sourceInfo, double timeoutSec);
    void onStreamingLost(const StreamingInfo& streamingInfo, double frameAgeSec);

    // Recording warnings and errors
    void onRecordingWarning(const QString& message);
    void onRecordingError(const QString& message);

#ifdef Q_OS_ANDROID
    void ensureCameraPermission();
#endif

    void updateSourceCompleter();
    void onSourceContextMenuRequested(const QPoint& pos);

    void onMediaInfoChanged(const MediaInfo& info);

    //
    // Media playback
    //

    void togglePause();
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void updatePlayPauseButton(bool paused);

    void onPlaybackPositionChanged(qint64 pos);

    void toggleTimeDisplayMode();
    void updateDurationLabel();

    void updateMediaBar();

    //
    // Audio
    //

    void volumeRequested(int value);
    void onVolumeChanged(float volume);
    void updateVolumeIcon(int volume, bool muted);

    void toggleMute();
    void onMutedChanged(bool muted);

    //
    // Fullscreen
    //

    void toggleFullscreen();
    void enterFullscreen();
    void leaveFullscreen();

    void positionFullscreenBar();
    void onActivityDetected(const QPoint& pos);
    void onIdle();

    void updateFullscreenBar();

    //
    // Shortcut actions
    //

    /**
     * @brief Seeks the current media by a relative time offset.
     *
     * The target position is clamped to the valid playback range before
     * requesting the seek. A HUD notification is displayed immediately
     * using the requested position to provide responsive user feedback,
     * while the actual playback position is updated asynchronously by
     * the media backend.
     *
     * @param deltaMs Relative seek offset, in milliseconds.
     */
    void stepPlayback(qint64 deltaMs);

    void stepVolume(int deltaPercent);

    //
    // Persistent settings
    //

    void loadPreferredFormats();
    void savePreferredFormats();

    void addSourceToHistory(const QUrl& url);
    void loadSourceHistory();
    void saveSourceHistory();

    void loadLastSourceType();
    void saveLastSourceType();

    static QByteArray loadSelectedCameraId();
    void saveSelectedCameraId();

    QString lastVideoDirectory() const;
    void saveLastVideoDirectory(const QString& directory);

    void applyInitialAudioSettings();
    void saveAudioSettings();

    //
    // Window title
    //

    void updateWindowTitle();
    QString buildSourceTitle() const;
    QString sourceTitle(const StreamingInfo& info) const;
    void appendStreamingInfo(QString& title, const StreamingInfo& info) const;

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

    AnimatedPushButton* openFileButton_{nullptr};
    QComboBox* urlCombo_{nullptr};
    QIcon clearHistoryIcon_;
    QWidget* fileUrlConfigWidget_{nullptr};

    QWidget* sourceConfigWidget_{nullptr};
    QStackedLayout* sourceConfigStack_{nullptr};

    RightPanelToggleButton* rightPanelToggle_{nullptr};

    QIcon settingsIcon_;
    AnimatedPushButton* settingsButton_{nullptr};

    QWidget* configRightBlockWidget_{nullptr};

    QIcon startIcon_;
    QIcon stopIcon_;
    AnimatedPushButton* toggleStreamingButton_{nullptr};

    QPushButton* recordingButton_{nullptr};
    QIcon stoppedIcon_;
    QIcon recordingIcon_;
    QIcon drainingIcon_;

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

    bool restartPending_{false};

    QHash<QByteArray, QCameraFormat> preferredFormats_;

    bool isUpdatingUi_{false};

    QString downscaleTitleStr_;
    QString sourceTitleStr_;

    SourceConfig sourceConfig_{};

    LastReportedError lastReportedError_{};

    StreamingInfo streamingInfo_{};
    MediaInfo mediaInfo_{};

    QWidget* mediaControlsWidget_{nullptr};

    AnimatedPushButton* playPauseButton_{nullptr};
    QIcon resumeIcon_;
    QIcon pauseIcon_;

    // Playback

    TimelineSlider* playbackSlider_{nullptr};

    QLabel* playbackPositionLabel_{nullptr};
    ClickableLabel* playbackDurationLabel_{nullptr};

    // Audio

    QPushButton* volumeButton_{nullptr};
    VolumeSlider* volumeSlider_{nullptr};
    QLabel* volumeLabel_{nullptr};
    QMenu* volumeMenu_{nullptr};

    QIcon volumeMuteIcon_;
    QIcon volumeLowIcon_;
    QIcon volumeMediumIcon_;
    QIcon volumeHighIcon_;

    QTimer saveAudioSettingsTimer_;

    QWidget* controlBar_{nullptr};

    // Fullscreen

    bool isFullScreen_{false};

    FullscreenVideoControlBar* fullscreenBar_{nullptr};
    QGraphicsOpacityEffect* fullscreenOpacity_{nullptr};

    QPropertyAnimation* showAnimation_{nullptr};
    QPropertyAnimation* hideAnimation_{nullptr};

    QIcon startIconLight_;
    QIcon stopIconLight_;

    QIcon resumeIconLight_;
    QIcon pauseIconLight_;

    TimeDisplayMode timeDisplayMode_{TimeDisplayMode::TotalDuration};
    QLabel* playbackSeparatorLabel_{nullptr};

    VideoShortcutManager shortcutManager_;

    QLabel* recordingStatsLabel_{nullptr};
};

} // namespace fluvel
