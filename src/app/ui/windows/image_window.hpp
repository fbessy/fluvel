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

#include "display_settings_widget.hpp"

#include "phi_editor.hpp"
#include "phi_view_model.hpp"
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
    void onDisplayedImageReady(const QImage& input);
    void onFileSelected(const QString& path);
    void onStateChanged(ofeli_app::WorkerState state);
    void onCameraWindowShown();
    void onCameraWindowClosed();

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
    CameraWindow* camera_window = nullptr;
    AnalysisWindow* evaluation_window = nullptr;
    SettingsWindow* settings_window = nullptr;
    AboutWindow* about_window = nullptr;
    LanguageWindow* language_window = nullptr;

    QPushButton* restartButton = nullptr;
    QPushButton* togglePauseButton = nullptr;
    QPushButton* stepButton = nullptr;
    QPushButton* convergeButton = nullptr;
    QPushButton* rightPanelToggle = nullptr;
    QPushButton* settingsButton = nullptr;

    ImageView* imageView = nullptr;
    AlgoInfoOverlay* imageOverlay = nullptr;

    DisplaySettingsWidget* displayBar = nullptr;

    QAction* openAct = nullptr;
    QAction* separatorAct = nullptr;
    static constexpr qsizetype MaxRecentFiles = 5;
    QAction* recentFileActs[MaxRecentFiles];
    QAction* deleteAct = nullptr;
    QAction* saveAct = nullptr;
    QAction* quitAct = nullptr;

    QAction* imageSessionAct = nullptr;
    QAction* cameraSessionAct = nullptr;
    QMediaDevices* mediaDevices = nullptr;
    QAction* analysisAct = nullptr;
    QAction* settingsAct = nullptr;

    QAction* aboutAct = nullptr;
    QAction* languageAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* segmentationMenu = nullptr;
    QMenu* sessionMenu = nullptr;
    QMenu* helpMenu = nullptr;

    QStringList nameFilters;
    QString last_directory_used;

    // --- Controllers / Workers ---
    ImageController* imageController = nullptr;
    std::unique_ptr<PhiEditor> phiEditor;
    std::unique_ptr<PhiViewModel> phiViewModel;

    QString m_fileName;
    QString m_fullPath;
    QSize   m_imageSize;
    int     m_channels = 0;

    QIcon startResumeIcon;
    QIcon restartIcon;
    QIcon pauseIcon;
    QIcon settingsIcon;

signals:

    void fileSelected(QString fileName);
};

}

#endif // IMAGE_WINDOW_H
