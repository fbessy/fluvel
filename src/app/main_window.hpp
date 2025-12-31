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
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

#include "application_settings.hpp"

#include "settings_window.hpp"
#include "evaluation_window.hpp"
#include "camera_window.hpp"
#include "about_window.hpp"
#include "language_window.hpp"

#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"

#include "active_contour.hpp"


QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QStackedWidget;
class QSpinBox;
class QSlider;
class QMimeData;
QT_END_NAMESPACE

//! [0]
namespace ofeli_gui
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public :

    MainWindow(ApplicationSettings& config);
    void set_zoom_factor(int val);
    int get_zoom_factor() const;

private slots :

    void open();
    //void onCameraChanged(int index);
    void openFileName();
    void openRecentFile();
    void deleteList();
    void saveImage();
    void print();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void start();
    void stop();
    void display_settings();
    void language();
    void doc();

    void do_scale0(int value);

    void adjustVerticalScroll(int min, int max);
    void adjustHorizontalScroll(int min, int max);

    void wheel_zoom(int,ScrollAreaWidget*);

    void onStartCameraActionTriggered();

private :

    /////////////////////////////////////////////////
    //        variables liés à la fenêtre          //
    /////////////////////////////////////////////////

    void initialize_active_contour();
    //void try_graphics();

    const unsigned char* img1_filtered;
    void put_displayed_active_contour();
    void erase_displayed_active_contour(const unsigned char* img, unsigned char min, unsigned char max);

    void updateCameraAction();

    QAction* startCameraAction = nullptr;
    QMediaDevices* mediaDevices = nullptr;

    ApplicationSettings& config;

    PixmapWidget* imageLabel;
    ScrollAreaWidget* scrollArea;

    QLabel* Cin_text;
    QLabel* Cout_text;
    QLabel* threshold_text;
    QStackedWidget* stackedWidget;

    QLabel* state_text;
    QLabel* iter_text;
    QLabel* changes_text;
    QLabel* quantile_text;
    QLabel* centroids_text;

    QLabel* time_text;

    QLabel* pixel_text;
    QLabel* phi_text;
    QLabel* lists_text;

    QSpinBox* scale_spin0;
    QSlider* scale_slider0;

    SettingsWindow* settings_window;
    EvaluationWindow* evaluation_window;
    CameraWindow* camera_window;
    AboutWindow* about_window;
    LanguageWindow* language_window;

    /////////////////////////////////////////////////
    //        variables liés au slot open()        //
    /////////////////////////////////////////////////

    // chaîne de caractère du chemin+nom de l'image obtenu a partir d'une boite de dialogue ouverture de Qt
    QString fileName;
    // buffer et informations des images
    // buffer 8 bits
    unsigned char* img1;
    bool is_rgb1;
    QImage::Format image_format;
    // colonne
    int img_width;
    // ligne
    int img_height;
    // taille de l'image
    int img_size;
    int find_offset(int x, int y) const;

    ///////////////////////////////////////////////////
    //    objets et variable liés au slot start()    //
    ///////////////////////////////////////////////////

    // objet filtre
    ofeli_ip::ActiveContour* ac;

    QImage image_result;
    unsigned char* image_result_uchar;
    QImage image_save_preprocess;

    QImage img;

    virtual void closeEvent(QCloseEvent* event) override;


    /////////////////////////////////////////////////////
    //   variables pour les events souris et claviers  //
    /////////////////////////////////////////////////////

    // pour la fenêtre principale
    bool has_click_stopping;
    bool has_step_by_step;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    void infinite_loop();
    bool has_algo_breaking;

    // filtre des evenements pour avoir la position que sur l'image et pas la fenêtre
    virtual bool eventFilter(QObject* object, QEvent* event) override;
    // position du curseur souris
    int positionX;
    int positionY;
    void show_phi_list_value();

    bool is_input_image_displayed;

    QStringList nameFilters;
    QString last_directory_used;
    void setCurrentFile(const QString &fileName1);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);
    QString curFile;

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;



    QAction* openAct;
    QAction* separatorAct;
    enum { MaxRecentFiles = 5 };
    QAction* recentFileActs[MaxRecentFiles];
    QAction* deleteAct;
    QAction* saveAct;
    QAction* printAct;
    QAction* cameraAct;
    QAction* exitAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* normalSizeAct;
    QAction* startAct;
    QAction* evaluateAct;
    QAction* settingsAct;
    QAction* aboutAct;
    QAction* languageAct;
    QAction* docAct;
    QAction* aboutQtAct;

    QMenu* fileMenu;
    QMenu* viewMenu;
    QMenu* segmentationMenu;
    QMenu* helpMenu;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

signals :

    void changed(const QMimeData* mimeData = 0);
    void signal_open();

};

inline int MainWindow::find_offset(int x, int y) const
{
    return x+y*img_width;
}

}

#endif // MAIN_WINDOW_HPP

//! \class ofeli::MainWindow
//! The class MainWindow is a graphical user interface using the Qt4 framework (QtCore and QtGui) to show an image with the active contour, to configure the parameters of the active contour and optionally the preprocessing step and to evaluate two segmentation results thanks to the evaluation window.
//! It based on the Qt example of the documentation http://doc.qt.nokia.com/4.7/widgets-imageviewer.html.
