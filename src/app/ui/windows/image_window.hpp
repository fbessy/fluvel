// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "about_window.hpp"
#include "analysis_window.hpp"
#include "camera_window.hpp"
#include "language_window.hpp"
#include "settings_window.hpp"

#include "active_contour_worker.hpp"
#include "display_settings_widget.hpp"
#include "right_panel_toggle_button.hpp"

#include <QMainWindow>

namespace ofeli_app
{

class ImageView;
class AlgoInfoOverlay;
class ImageController;
class ActiveContourWorker;
struct AlgoStats;

class ImageWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImageWindow(QWidget* parent = nullptr);

public slots:
    void onInputImageReady(const QImage& inputImage);
    void onDisplayedImageReady(const QImage& displayed);
    void onFileOpened(const QString& path);
    void onStateChanged(ofeli_app::WorkerState state);
    void onCameraWindowShown();
    void onCameraWindowClosed();

signals:

    void fileSelected(QString fileName);
    void inputImageReady(const QImage& inputImage);
    void imageDropped(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // --- Setup ---
    void setupUi();
    void setupActions();
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
    CameraWindow* camera_window_ = nullptr;
    AnalysisWindow* evaluation_window_ = nullptr;
    SettingsWindow* settings_window_ = nullptr;
    AboutWindow* about_window_ = nullptr;
    LanguageWindow* language_window_ = nullptr;

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

    QString m_fileName_;
    QString m_fullPath_;
    QSize m_imageSize_;
    int m_channels_ = 0;

    QIcon startResumeIcon_;
    QIcon restartIcon_;
    QIcon pauseIcon_;
    QIcon settingsIcon_;
};

QString buildImageFilter();
bool isSupportedImage(const QString& path);

} // namespace ofeli_app
