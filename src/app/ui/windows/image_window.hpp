// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_worker.hpp"

#include <QIcon>
#include <QImage>
#include <QMainWindow>
#include <QString>

#include <array>

class QWidget;

class QMenu;
class QAction;
class QPushButton;

class QShowEvent;
class QCloseEvent;

namespace fluvel_app
{

class CameraWindow;
class AnalysisWindow;
class SettingsWindow;
class AboutWindow;
class LanguageWindow;

class RightPanelToggleButton;
class DisplaySettingsWidget;

class ImageViewerWidget;
class ImageController;

/**
 * @brief Main application window for image processing and segmentation.
 *
 * This window provides the primary user interface to:
 * - open and display images
 * - control segmentation and processing workflows
 * - manage sessions (image, camera, analysis)
 * - access application settings and tools
 *
 * It coordinates:
 * - ImageViewerWidget for rendering
 * - ImageController for processing logic
 * - multiple child windows (camera, analysis, settings, etc.)
 *
 * The class also manages recent files, user actions, and application state.
 */
class ImageWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the main image window.
     *      * @param parent Optional parent widget.
     */
    explicit ImageWindow(QWidget* parent = nullptr);

signals:
    /**
     * @brief Emitted when a file is selected.
     *      * @param fileName Path to the selected file.
     */
    void fileSelected(const QString& fileName);

    /**
     * @brief Emitted when an image is dropped into the window.
     *      * @param path Path to the dropped file.
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
    void onStateChanged(fluvel_app::WorkerState state);
    void onCameraWindowShown();
    void onCameraWindowClosed();

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

    void updateCameraAction(bool available);
    void onStartCameraActionTriggered();

    void saveDisplayed();

    // --- UI ---
    CameraWindow* cameraWindow_ = nullptr;
    AnalysisWindow* analysisWindow_ = nullptr;
    SettingsWindow* settingsWindow_ = nullptr;
    AboutWindow* aboutWindow_ = nullptr;
    LanguageWindow* languageWindow_ = nullptr;

    QMenu* fileMenu_ = nullptr;
    QMenu* segmentationMenu_ = nullptr;
    QMenu* sessionMenu_ = nullptr;
    QMenu* helpMenu_ = nullptr;

    QPushButton* restartButton_ = nullptr;
    QPushButton* togglePauseButton_ = nullptr;
    QPushButton* stepButton_ = nullptr;
    QPushButton* convergeButton_ = nullptr;
    RightPanelToggleButton* rightPanelToggle_ = nullptr;
    QPushButton* settingsButton_ = nullptr;

    QIcon startResumeIcon_;
    QIcon restartIcon_;
    QIcon pauseIcon_;
    QIcon settingsIcon_;

    DisplaySettingsWidget* displayBar_ = nullptr;

    // --- Actions ---

    QAction* openAct_ = nullptr;
    QAction* separatorAct_ = nullptr;

    static constexpr qsizetype kMaxRecentFiles{5};
    std::array<QAction*, kMaxRecentFiles> recentFileActs_;

    QAction* clearAct_ = nullptr;
    QAction* saveAct_ = nullptr;
    QAction* quitAct_ = nullptr;

    QAction* imageSessionAct_ = nullptr;
    QAction* cameraSessionAct_ = nullptr;
    QAction* analysisAct_ = nullptr;
    QAction* settingsAct_ = nullptr;

    QAction* aboutAct_ = nullptr;
    QAction* languageAct_ = nullptr;

    // --- VIEW - CONTROLLER ---
    ImageViewerWidget* imageViewer_ = nullptr;
    ImageController* imageController_ = nullptr;

    // --- Current image state ---
    QString fileName_;
    QString fullPath_;
    QSize inputSize_;
    int channels_{0};
    DownscaleParams currentDownscale_{};

    QString lastDirectoryUsed_;
};

} // namespace fluvel_app
