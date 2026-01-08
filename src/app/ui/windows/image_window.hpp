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

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>

#include "camera_window.hpp"
#include "evaluation_window.hpp"
#include "settings_window.hpp"
#include "about_window.hpp"
#include "language_window.hpp"

namespace ofeli_gui {

class ImageViewBase;
class ImagePipelineController;
class ActiveContourWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    virtual void closeEvent(QCloseEvent* event) override;

    // --- UI ---
    CameraWindow* camera_window = nullptr;
    EvaluationWindow* evaluation_window = nullptr;
    SettingsWindow* settings_window = nullptr;
    AboutWindow* about_window = nullptr;
    LanguageWindow* language_window = nullptr;

    QPushButton* startButton = nullptr;
    QPushButton* pauseButton = nullptr;
    QPushButton* stepButton = nullptr;

    ImageViewBase* imageView = nullptr;




    QAction* openAct = nullptr;
    QAction* separatorAct = nullptr;
    enum { MaxRecentFiles = 5 };
    QAction* recentFileActs[MaxRecentFiles];
    QAction* deleteAct = nullptr;
    QAction* saveAct = nullptr;
    QAction* exitAct = nullptr;

    QAction* cameraAct = nullptr;
    QMediaDevices* mediaDevices = nullptr;
    QAction* evaluateAct = nullptr;
    QAction* settingsAct = nullptr;

    QAction* aboutAct = nullptr;
    QAction* languageAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* windowMenu = nullptr;
    QMenu* helpMenu = nullptr;

    QStringList nameFilters;
    QString last_directory_used;
    void setCurrentFile(const QString &fileName1);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    void updateCameraAction();
    void onStartCameraActionTriggered();



    // --- Controllers / Workers ---
    ImagePipelineController* imageController = nullptr;
    ActiveContourWorker* acWorker = nullptr;

    // --- Setup ---
    void setupUi();
    void setupActions();
    void setupConnections();

signals:

    void fileSelected(QString fileName);

    /*
    // Actions UI (fines, lisibles)
    void openImage();
    void saveImage();

    void startContour(); // start or restart contour
    void suspendResumeContour();
    void suspendToNextIteration();*/
};

}

#endif // MAIN_WINDOW_HPP
