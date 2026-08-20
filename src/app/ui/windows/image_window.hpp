// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_processing_worker.hpp"
#include "analysis_window.hpp"
#include "video_window.hpp"

#include <QIcon>
#include <QImage>
#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <QTimer>

#include <array>
#include <memory>

class QWidget;

class QMenu;
class QAction;

class QGraphicsOpacityEffect;
class QPropertyAnimation;

class QShowEvent;
class QCloseEvent;

class QLabel;

namespace fluvel
{

/**
 * @brief Set of control buttons associated with an active contour session.
 *
 * This structure groups together the UI controls used to drive the
 * active contour execution. It is used by both the standard toolbar
 * and the fullscreen control overlay.
 */
struct ControlButtons
{
    QPushButton* restart{nullptr};
    QPushButton* pause{nullptr};
    QPushButton* step{nullptr};
    QPushButton* converge{nullptr};
};

/**
 * @brief Visual presentation of a control button set.
 *
 * A control theme combines:
 * - the target buttons to update
 * - the icons to display
 * - presentation options such as text and tooltip visibility
 *
 * This allows the same update logic to be reused for both
 * the standard toolbar and the fullscreen overlay controls.
 */
struct ControlTheme
{
    ControlButtons buttons;

    bool showText{true};
    bool showToolTips{true};
};

class SettingsDialog;
class AboutDialog;
class PreferencesDialog;

class ConfigurationActionsWidget;
class DisplaySettingsWidget;
class FullscreenImageControlBar;

class CaptureController;
class CaptureControlsWidget;

class ImageViewerWidget;
class ImageController;

/**
 * @brief Main window for image-based active contour processing.
 *
 * ImageWindow provides the primary user interface for image
 * segmentation workflows and acts as the entry point to the
 * image session.
 *
 * It coordinates image loading, active contour execution,
 * visualization and application settings.
 *
 * The window can also launch the independent video session
 * through VideoWindow.
 */
class ImageWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the main image window.
     *
     * @param parent Optional parent widget.
     */
    explicit ImageWindow(QWidget* parent = nullptr);

signals:
    /**
     * @brief Emitted when a file is selected.
     *
     * @param fileName Path to the selected file.
     */
    void fileSelected(const QString& fileName);

    /**
     * @brief Emitted when an image is dropped into the window.
     *
     * @param path Path to the dropped file.
     */
    void imageDropped(const QString& path);

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
    void onInputImageReady(const QImage& input);
    void onFileOpened(const QString& path);
    void onStateChanged(fluvel::WorkerState state);
    void onVideoWindowShown();
    void onVideoWindowClosed();

    // --- Setup ---
    void setupUi();
    void setupActions();
    void setupMenus();
    void setupControllers();
    void setupChildWindows();

    void applyInitialSettings();

    void setupConnections();
    void setupUserActionsConnections();
    void setupFileEventConnections();

    void bindApplicationSettingsToController();
    void bindApplicationSettingsToView();
    void bindUiToApplicationSettings();

    void updateWindowTitle();
    void setCurrentFile(const QString& fileName);
    void updateRecentFileActions();
    void clearRecentFiles();

    void showErrorMessage(const QString& msg);
    void showWarningMessage(const QString& msg);

    void onImageSessionActionTriggered();
    void onStartVideoActionTriggered();

    void toggleFullscreen();
    void enterFullscreen();
    void leaveFullscreen();

    void positionFullscreenBar();
    void updateButtonState(const ControlTheme& theme, WorkerState state);
    void updateButtonIcons(WorkerState state);
    void onActivityDetected(const QPoint& pos);
    void onIdle();

#ifdef FLUVEL_USE_FFMPEG
    void onRecordingStatsChanged(const RecorderStats& stats);
    void onRecordingStarted(const QString& outputPath);
    void onRecordingFinalized(const QString& outputPath);
    void onRecordingWarning(const QString& message);
    void onRecordingError(const QString& message);

    QLabel* recordingStatsLabel_{nullptr};
#endif

    void onSnapshotSaved(const QString& filename);
    void onSnapshotError(const QString& message);

    // --- UI ---
    std::unique_ptr<VideoWindow> videoWindow_;
    std::unique_ptr<AnalysisWindow> analysisWindow_;
    SettingsDialog* settingsDialog_{nullptr};
    AboutDialog* AboutDialog_{nullptr};

    QMenu* fileMenu_{nullptr};
    QMenu* segmentationMenu_{nullptr};
    QMenu* sessionMenu_{nullptr};
    QMenu* helpMenu_{nullptr};

    AnimatedPushButton* restartButton_{nullptr};
    AnimatedPushButton* togglePauseButton_{nullptr};
    AnimatedPushButton* stepButton_{nullptr};
    AnimatedPushButton* convergeButton_{nullptr};

    ConfigurationActionsWidget* configurationActions_{nullptr};

    QWidget* controlBar_{nullptr};

    QIcon startResumeIcon_;
    QIcon restartIcon_;
    QIcon pauseIcon_;

    /// Icons used by the fullscreen control bar.
    QIcon startResumeIconFs_;
    QIcon restartIconFs_;
    QIcon pauseIconFs_;

    DisplaySettingsWidget* displayBar_{nullptr};

    // --- Actions ---

    QAction* openAct_{nullptr};
    QAction* separatorAct_{nullptr};

    static constexpr qsizetype kMaxRecentFiles{5};
    std::array<QAction*, kMaxRecentFiles> recentFileActs_;

    QAction* clearAct_{nullptr};
    QAction* quitAct_{nullptr};

    QAction* imageSessionAct_{nullptr};
    QAction* videoSessionAct_{nullptr};
    QAction* analysisAct_{nullptr};

    QAction* aboutAct_{nullptr};

    // --- VIEW - CONTROLLER ---
    ImageViewerWidget* imageViewer_{nullptr};
    ImageController* imageController_{nullptr};

    // --- Current image state ---
    QString fileName_;
    QString fullPath_;
    QSize inputSize_;
    int channels_{0};
    DownscaleParams currentDownscale_{};

    QString lastDirectoryUsed_;

    bool isFullScreen_{false};

    FullscreenImageControlBar* fullscreenBar_{nullptr};

    QGraphicsOpacityEffect* fullscreenOpacity_{nullptr};

    QPropertyAnimation* showAnimation_{nullptr};
    QPropertyAnimation* hideAnimation_{nullptr};

    ControlTheme normalTheme_;
    ControlTheme fullscreenTheme_;

    CaptureController* captureController_{nullptr};
    CaptureControlsWidget* captureWidget_{nullptr};
    QImage captureImage_;
};

} // namespace fluvel
