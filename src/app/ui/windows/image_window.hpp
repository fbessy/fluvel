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

class ImageView;
class ImageController;

class ImageWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImageWindow(QWidget* parent = nullptr);

signals:
    void fileSelected(const QString& fileName);
    void imageDropped(const QString& path);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void onDisplayedImageReady(const QImage& displayed);
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
    static QString strippedName(const QString& fullFileName);

    void showErrorMessage(const QString& msg);

    void updateCameraAction(bool available);
    void onStartCameraActionTriggered();

    void saveDisplayed();
    static QString makeUniqueFileName(const QString& filePath);

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

    static constexpr qsizetype kMaxRecentFiles = 5;
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
    ImageView* imageView_ = nullptr;
    ImageController* imageController_ = nullptr;

    // --- Current image state ---
    QString fileName_;
    QString fullPath_;
    QSize imageSize_;
    int channels_{0};

    QString lastDirectoryUsed_;
};

QString buildImageFilter();
bool isSupportedImage(const QString& path);

} // namespace fluvel_app
