// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_worker.hpp"

#include <QImage>
#include <QMainWindow>
#include <QString>

class QWidget;
class QShowEvent;
class QCloseEvent;
class QPushButton;
class QMediaDevices;

namespace fluvel_app
{

class ImageView;
class ImageController;

class CameraWindow;
class AnalysisWindow;
class SettingsWindow;
class AboutWindow;
class LanguageWindow;

class RightPanelToggleButton;
class DisplaySettingsWidget;

class ImageWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImageWindow(QWidget* parent = nullptr);

signals:
    void fileSelected(QString fileName);
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
    void bindApplicationSettings();
    void setupConnections();

    void updateWindowTitle();
    void setCurrentFile(const QString& fileName);
    void updateRecentFileActions();
    void deleteList();
    static QString strippedName(const QString& fullFileName);

    void showErrorMessage(const QString& msg);

    void updateCameraAction();
    void onStartCameraActionTriggered();

    void saveDisplayed();
    static QString makeUniqueFileName(const QString& filePath);

    // --- UI ---
    CameraWindow* cameraWindow_ = nullptr;
    AnalysisWindow* evaluationWindow_ = nullptr;
    SettingsWindow* settingsWindow_ = nullptr;
    AboutWindow* aboutWindow_ = nullptr;
    LanguageWindow* languageWindow_ = nullptr;

    QPushButton* restartButton_ = nullptr;
    QPushButton* togglePauseButton_ = nullptr;
    QPushButton* stepButton_ = nullptr;
    QPushButton* convergeButton_ = nullptr;
    RightPanelToggleButton* rightPanelToggle_ = nullptr;
    QPushButton* settingsButton_ = nullptr;

    ImageView* imageView_ = nullptr;

    DisplaySettingsWidget* displayBar_ = nullptr;

    QAction* openAct_ = nullptr;
    QAction* separatorAct_ = nullptr;

    static constexpr qsizetype kMaxRecentFiles = 5;
    QAction* recentFileActs_[kMaxRecentFiles];

    QAction* deleteAct_ = nullptr;
    QAction* saveAct_ = nullptr;
    QAction* quitAct_ = nullptr;

    QAction* imageSessionAct_ = nullptr;
    QAction* cameraSessionAct_ = nullptr;
    QMediaDevices* mediaDevices_ = nullptr;
    QAction* analysisAct_ = nullptr;
    QAction* settingsAct_ = nullptr;

    QAction* aboutAct_ = nullptr;
    QAction* languageAct_ = nullptr;

    QMenu* fileMenu_ = nullptr;
    QMenu* segmentationMenu_ = nullptr;
    QMenu* sessionMenu_ = nullptr;
    QMenu* helpMenu_ = nullptr;

    QString last_directory_used_;

    // --- Controllers / Workers ---
    ImageController* imageController_ = nullptr;

    QString fileName_;
    QString fullPath_;
    QSize imageSize_;
    int channels_{0};

    QIcon startResumeIcon_;
    QIcon restartIcon_;
    QIcon pauseIcon_;
    QIcon settingsIcon_;
};

QString buildImageFilter();
bool isSupportedImage(const QString& path);

} // namespace fluvel_app
