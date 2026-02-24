/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef IMAGE_WINDOW_HPP
#define IMAGE_WINDOW_HPP

#include <QMainWindow>

#include "camera_window.hpp"
#include "analysis_window.hpp"
#include "settings_window.hpp"
#include "about_window.hpp"
#include "language_window.hpp"

#include "right_panel_toggle_button.hpp"
#include "display_settings_widget.hpp"
#include "active_contour_worker.hpp"

namespace ofeli_app {

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
    void refreshAlgoOverlay();
    void onInputImageReady(const QImage& inputImage);
    void onDisplayedImageReady(const QImage& displayed);
    void onFileSelected(const QString& path);
    void onStateChanged(ofeli_app::WorkerState state);
    void onCameraWindowShown();
    void onCameraWindowClosed();

signals:

    void fileSelected(QString fileName);
    void inputImageReady(const QImage& inputImage);

protected:
    void closeEvent(QCloseEvent* event) override;

private:

    // --- Setup ---
    void setupUi();
    void setupActions();
    void setupConnections();

    void updateWindowTitle();
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    void deleteList();
    static QString strippedName(const QString &fullFileName);

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
    AlgoInfoOverlay* imageOverlay_ = nullptr;

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

    QStringList nameFilters_;
    QString last_directory_used_;

    // --- Controllers / Workers ---
    ImageController* imageController_ = nullptr;


    QString m_fileName_;
    QString m_fullPath_;
    QSize   m_imageSize_;
    int     m_channels_ = 0;

    QIcon startResumeIcon_;
    QIcon restartIcon_;
    QIcon pauseIcon_;
    QIcon settingsIcon_;
};

}

#endif // IMAGE_WINDOW_H
