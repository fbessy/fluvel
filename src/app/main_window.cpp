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

// origin code from Qt exemple "imageviewer"
// http://doc.qt.nokia.com/4.7/widgets-imageviewer.html

////////////////////////// Ofeli /////////////////////////////

// Qt GUI part of Ofeli

#include "main_window.hpp"
#include "contour_rendering.hpp"

                    /////////////////////

// image processing part of Ofeli
#include "region_ac.hpp"
#include "region_color_ac.hpp"
#include "edge_ac.hpp"

//////////////////////////////////////////////////////////////

#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include <QMediaDevices>

#include <ctime>         // for std::clock_t, std::clock() and CLOCKS_PER_SEC
#include <cstring>       // for std::memcpy
// if you want a partial support of DICOM image, without support of JPEG-2000 encapsulated image
// install DCMTK library to the project
// uncomment row 72, 1002 and the block at 1074.
//#include "dcmtk/dcmimgle/dcmimage.h"
#include "matrix.hpp"
//#include "contour_item.hpp"

namespace ofeli_gui
{

MainWindow::MainWindow() :
    scale_spin0(nullptr), scale_slider0(nullptr),
    img1(nullptr), img_width(0), img_height(0),
    ac(nullptr), image_result_uchar(nullptr),
    positionX(0), positionY(0),
    settings_window(nullptr),
    evaluation_window(nullptr),
    camera_window(nullptr),
    about_window(nullptr),
    language_window(nullptr)
{
    setWindowTitle( tr("Ofeli") );

#ifdef Q_OS_LINUX
    setWindowIcon( QIcon(":/icons/app/Ofeli.svg") );
#endif

    QSettings settings;

    const auto geo = settings.value("Main/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    //////////////////////////////////////////////////////////////////////////

    imageView = new ImageViewBase(this);

    //////////////////////////////////////////////////////////////////////////

    Cin_text = new QLabel(this);
    Cin_text->setText("Cin =");
    Cin_text->setAlignment(Qt::AlignRight);
    Cout_text = new QLabel(this);
    Cout_text->setText("Cout =");
    Cout_text->setToolTip(tr("outside average of the image"));
    Cout_text->setAlignment(Qt::AlignRight);
    Cin_text->setToolTip(tr("inside average of the image"));
    threshold_text = new QLabel(this);
    threshold_text->setToolTip(tr("0.7 times this threshold before to begin the steepest descent"));
    threshold_text->setAlignment(Qt::AlignRight);

    QGroupBox* speed1_group = new QGroupBox(tr("Chan-Vese model information"));
    QVBoxLayout* chanvese_text = new QVBoxLayout;
    chanvese_text->addWidget(Cout_text);
    chanvese_text->addWidget(Cin_text);
    speed1_group->setLayout(chanvese_text);

    QGroupBox* speed2_group = new QGroupBox(tr("Geodesic model information"));
    QVBoxLayout* threshold_layout = new QVBoxLayout;
    threshold_layout->addWidget(threshold_text);
    speed2_group->setLayout(threshold_layout);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(speed1_group);
    stackedWidget->addWidget(speed2_group);

    //////////////////////////////////////////////////////////////////////////

    state_text = new QLabel(this);
    state_text->setText(tr("state ="));
    state_text->setToolTip(tr("Evolution state of the active contour for a current iteration.)"));
    state_text->setAlignment(Qt::AlignRight);
    iter_text = new QLabel(this);
    iter_text->setText(tr("iteration t ="));
    iter_text->setToolTip(tr("must be strictly less than iteration_max = 5 × max(img_width,img_height)"));
    iter_text->setAlignment(Qt::AlignRight);
    changes_text = new QLabel(this);
    changes_text->setText(tr("lists changes ="));
    changes_text->setToolTip(tr("Updated only for each iteration of the cycle 1."));
    changes_text->setAlignment(Qt::AlignRight);
    quantile_text = new QLabel(this);
    quantile_text->setText(tr("80th Hausdorff quantile ="));
    quantile_text->setToolTip(tr("Updated only at the end of some cycle 2. It is a normalized value in percent in function of the diagonal size of the image."));
    quantile_text->setAlignment(Qt::AlignRight);
    centroids_text = new QLabel(this);
    centroids_text->setText(tr("centroid evolution ="));
    centroids_text->setToolTip(tr("Updated only at the end of some cycle 2. It is a normalized value in percent in function of the diagonal size of the image."));
    centroids_text->setAlignment(Qt::AlignRight);

    QGroupBox* stop_group = new QGroupBox(tr("State and evolution data"));
    QVBoxLayout* condi_layout = new QVBoxLayout;
    condi_layout->addWidget(state_text);
    condi_layout->addWidget(iter_text);
    condi_layout->addWidget(changes_text);
    condi_layout->addWidget(quantile_text);
    condi_layout->addWidget(centroids_text);
    stop_group->setLayout(condi_layout);

    //////////////////////////////////////////////////////////////////////////

    pixel_text = new QLabel(this);
    pixel_text->setMinimumWidth(248);
    pixel_text->setText(tr("img(x,y) ="));
    pixel_text->setToolTip(tr("pixel value at the position (x,y)"));
    pixel_text->setAlignment(Qt::AlignRight);
    phi_text = new QLabel(this);
    phi_text->setText("ϕ(x,y) =");
    phi_text->setToolTip(tr("level set function value at the position (x,y), value ∈ { -3, -1, 1, 3 }"));
    phi_text->setAlignment(Qt::AlignRight);
    lists_text = new QLabel(this);
    lists_text->setText("Lin & Lout");
    lists_text->setAlignment(Qt::AlignRight);

    QGroupBox* data_group = new QGroupBox(tr("Data, level set function and lists"));
    QVBoxLayout* data_layout = new QVBoxLayout;
    data_layout->addWidget(pixel_text);
    data_layout->addWidget(phi_text);
    data_layout->addWidget(lists_text);
    data_group->setLayout(data_layout);

    //////////////////////////////////////////////////////////////////////////

    time_text = new QLabel(this);
    time_text->setText(tr("time ="));
    time_text->setToolTip(tr("elapsed time in the loop, without the initialization time when the constructor is called"));
    time_text->setAlignment(Qt::AlignRight);

    QGroupBox* time_group = new QGroupBox(tr("Elapsed time"));
    QVBoxLayout* elapsed_layout = new QVBoxLayout;
    elapsed_layout->addWidget(time_text);
    time_group->setLayout(elapsed_layout);

    //////////////////////////////////////////////////////////////////////////

    scale_spin0 = new QSpinBox;
    scale_spin0->setSingleStep(25);
    scale_spin0->setMinimum(1);
    scale_spin0->setMaximum(5000);
    scale_spin0->setSuffix(" %");
    scale_spin0->installEventFilter(this);
    scale_spin0->setMouseTracking(true);
    scale_slider0 = new QSlider(Qt::Horizontal, this);

#ifdef Q_OS_LINUX
    scale_slider0->setTickPosition(QSlider::TicksBelow);
#else
    scale_slider0->setTickPosition(QSlider::TicksAbove);
#endif

    scale_slider0->setMinimum(1);
    scale_slider0->setMaximum(1000);
    scale_slider0->setTickInterval(100);
    scale_slider0->setSingleStep(25);
    scale_slider0->setValue(100);
    scale_slider0->installEventFilter(this);
    scale_slider0->setMouseTracking(true);

    QFormLayout* scale_spin_layout = new QFormLayout;
    scale_spin_layout->addRow(tr("scale :"), scale_spin0);
    scale_spin_layout->setFormAlignment(Qt::AlignRight);
    QVBoxLayout* scale_layout = new QVBoxLayout;
    scale_layout->addLayout(scale_spin_layout);
    scale_layout->addWidget(scale_slider0);
    QGroupBox* scale_group = new QGroupBox(tr("Display"));
    scale_group->setLayout(scale_layout);

    connect(scale_spin0,SIGNAL(valueChanged(int)),this,SLOT(do_scale0(int)));
    connect(scale_slider0,SIGNAL(valueChanged(int)),scale_spin0,SLOT(setValue(int)));
    connect(scale_spin0,SIGNAL(valueChanged(int)),scale_slider0,SLOT(setValue(int)));

    scale_spin0->setValue(settings.value("Settings/Display/zoom_factor", 100).toInt());

    //////////////////////////////////////////////////////////////////////////

    dockInfo = new QDockWidget(tr("Informations"), this);
    //dockInfo->setAllowedAreas(Qt::RightDockWidgetArea);

    addDockWidget(Qt::RightDockWidgetArea, dockInfo);
    //menuBar()->addMenu(tr("Affichage"))->addAction(dockInfo->toggleViewAction());

    QWidget* info = new QWidget(dockInfo);
    QVBoxLayout* info_layout = new QVBoxLayout(info);
    info_layout->addWidget(stackedWidget);
    info_layout->addWidget(stop_group);
    info_layout->addWidget(time_group);

    //info_layout->addWidget(data_group);
    //info_layout->addWidget(scale_group);
    info_layout->addStretch(1);

    dockInfo->setWidget(info);

    QHBoxLayout *layout_this = new QHBoxLayout;
    layout_this->addWidget(imageView,1);
    //layout_this->addLayout(info_layout,0);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    QLabel* widget_this = new QLabel;
    widget_this->setLayout(layout_this);

    setCentralWidget(widget_this);

    //////////////////////////////////////////////////////////////////////////

    // config is modified inside settings window when the user clicks on accept.
    settings_window = new SettingsWindow(this);

    evaluation_window = new EvaluationWindow(this);

    // config reference is const inside CameraWindow
    camera_window = new CameraWindow(this);

    // config.app_language reference is const inside AboutWindow
    about_window = new AboutWindow(this);

    language_window = new LanguageWindow(this);

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Actions                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    QStyle* style =  QApplication::style();

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an image file (*.png, *.bmp, *.jpg, *.jpeg, *.tiff, *.tif, *.gif, *.pbm, *.pgm, *.ppm, *.svg, *.svgz, *.mng, *.xbm, *.xpm)."));
    openAct->setEnabled(true);

    openAct->setIcon( QIcon::fromTheme( QIcon::ThemeIcon::DocumentOpen,
                                        style->standardIcon(QStyle::SP_DirOpenIcon)) );

    connect(openAct, SIGNAL(triggered()), this, SLOT(openFileName()));

    cameraAct = new QAction(tr("Camera"), this);
    cameraAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::CameraWeb) );
    cameraAct->setEnabled(false);

    connect(cameraAct, &QAction::triggered, this, &MainWindow::onStartCameraActionTriggered);

    mediaDevices = new QMediaDevices(this);
    connect(mediaDevices, &QMediaDevices::videoInputsChanged, this, &MainWindow::updateCameraAction);

    updateCameraAction();



    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);

        recentFileActs[i]->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpenRecent,
                                                     QIcon(":/icons/toolbar/document-open-recent.svg")) );

        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    deleteAct = new QAction(tr("Clear list"), this);
    deleteAct->setStatusTip(tr("Clean the recent files list."));

    deleteAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::EditClear,
                                         QIcon(":/icons/toolbar/edit-clear-list.svg")) );

    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteList()));

    saveAct = new QAction(tr("S&ave..."), this);
    saveAct->setShortcut(tr("Ctrl+A"));
    saveAct->setEnabled(false);
    saveAct->setStatusTip(tr("Save the displayed and preprocessed images."));

    saveAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs,
                                       style->standardIcon(QStyle::SP_DialogSaveButton)) );

    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveImage()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(false);

    printAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentPrint,
                                        QIcon(":/icons/toolbar/document-print.svg")) );

    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+X"));

    exitAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit,
                                       style->standardIcon(QStyle::SP_TitleBarCloseButton)) );

    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);

    zoomInAct->setIcon( QIcon::fromTheme( QIcon::ThemeIcon::ZoomIn,
                                          QIcon(":/icons/toolbar/zoom-in.svg") ) );

    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);

    zoomOutAct->setIcon( QIcon::fromTheme( QIcon::ThemeIcon::ZoomOut,
                                           QIcon(":/icons/toolbar/zoom-out.svg") ) );

    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+N"));
    normalSizeAct->setEnabled(false);

    normalSizeAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::ViewRestore,
                                             QIcon(":/icons/toolbar/zoom-original.svg")) );

    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    startAct = new QAction(tr("&Start"), this);
    startAct->setShortcut(tr("Ctrl+S"));
    startAct->setEnabled(false);
    startAct->setStatusTip(tr("Start the active contour."));

    startAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart,
                                        style->standardIcon(QStyle::SP_MediaPlay)) );

    connect(startAct, SIGNAL(triggered()), this, SLOT(start()));

    evaluateAct = new QAction(tr("E&valuate"), this);
    evaluateAct->setStatusTip(tr("Compute the modified Hausdorff distance."));
    evaluateAct->setShortcut(tr("Ctrl+V"));
    evaluateAct->setEnabled(true);

    evaluateAct->setIcon( QIcon::fromTheme("accessories-calculator",
                                           QIcon(":/icons/toolbar/insert-horizontal-rule.svg")) );

    connect( evaluateAct, SIGNAL(triggered()), evaluation_window, SLOT(show()) );

    settingsAct = new QAction(tr("S&ettings"), this);
    settingsAct->setStatusTip(tr("Image preprocessing and active contour initialization."));
    settingsAct->setShortcut(tr("Ctrl+E"));
    settingsAct->setEnabled(true);

    settingsAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties,
                                           QIcon(":/icons/toolbar/configure.svg")) );

    connect(settingsAct, SIGNAL(triggered()), this, SLOT(display_settings()));

    menuBar()->addSeparator();

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Information, license and home page."));

    aboutAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout,
                                        style->standardIcon(QStyle::QStyle::SP_MessageBoxInformation)) );

    connect(aboutAct, SIGNAL(triggered()), about_window, SLOT(show()));

    languageAct = new QAction(tr("&Language"), this);
    languageAct->setStatusTip(tr("Choose the application language."));

    languageAct->setIcon( QIcon::fromTheme("preferences-desktop-locale",
                                           QIcon(":/icons/toolbar/preferences-desktop-locale.svg")) );

    connect(languageAct, SIGNAL(triggered()), this, SLOT(language()));

    docAct = new QAction(tr("&Documentation"), this);
    docAct->setStatusTip(tr("Online developer's documentation."));

    docAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::HelpFaq,
                                      QIcon(":/icons/toolbar/help-contents.svg")) );

    connect(docAct, SIGNAL(triggered()), this, SLOT(doc()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Menus                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);

    separatorAct = fileMenu->addSeparator();
    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        fileMenu->addAction(recentFileActs[i]);
    }

    fileMenu->addAction(deleteAct);

    fileMenu->addSeparator();

    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    updateRecentFileActions();

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);

    windowMenu = new QMenu(tr("&Window"), this);
    windowMenu->addAction(cameraAct);
    windowMenu->addAction(evaluateAct);
    windowMenu->addAction(settingsAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(languageAct);
    helpMenu->addAction(docAct);
    //helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(windowMenu);
    menuBar()->addMenu(helpMenu);

    //////////////////////////////////////////////////////////////////////////

    nameFilters << "*.bmp"
                //<< "*.dcm"
                << "*.gif"
                << "*.jpg" << "*.jpeg" << "*.mng"
                << "*.pbm" << "*.png" << "*.pgm"
                << "*.ppm" << "*.svg" << "*.svgz"
                << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    nameFilters.removeDuplicates();

    last_directory_used = settings.value("Main/Name/last_directory_used",QDir().homePath()).toString();

    statusBar()->clearMessage();

    connect( this, SIGNAL(signal_open()), this, SLOT(stop()), Qt::DirectConnection );
    connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
}

void MainWindow::openFileName()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open File"),
                                            last_directory_used,
                                            tr("Image Files (%1)").arg(nameFilters.join(" ")));


    emit signal_open();
}

// Fonction appelée lorsqu'on ouvre une image
void MainWindow::open()
{
    if( !fileName.isEmpty() )
    {
        QFileInfo fi(fileName);

        // if you want a partial support of DICOM image, without support of JPEG-2000 encapsulated image
        // install DCMTK library to the project (there is the folder "external_libraries")
        // and uncomment below

        /*if( fi.completeSuffix() == "dcm" || fi.completeSuffix() == "DCM" )
        {
            DicomImage* dcm_img = new DicomImage( fileName.toStdString().c_str() );

            if( dcm_img != nullptr)
            {
                if( dcm_img->getStatus() == EIS_Normal )
                {
                    int width = dcm_img->getWidth();
                    int height = dcm_img->getHeight();
                    unsigned char* pixelData = (unsigned char*)( dcm_img->getOutputData(8) );

                    if( pixelData != nullptr )
                    {
                        img = QImage(width,height,QImage::Format_Indexed8);

                        for( int y = 0; y < height; y++ )
                        {
                            for( int x = 0; x < width; x++ )
                            {
                                *( img.scanLine(y)+x ) = pixelData[x+y*width];
                            }
                        }

                    }
                }
                else
                {
                    std::cerr << "Error: cannot load DICOM image (" << DicomImage::getString(dcm_img->getStatus()) << ")" << std::endl;
                }
            }
            delete dcm_img;
        }*/
        //else
        //{
            img = QImage(fileName);
            if( img.isNull() )
            {
                QMessageBox::information(this, tr("Opening error - Ofeli"),
                                         tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
                return;
            }
        //}

        image_format = img.format();

        if(    image_format == QImage::Format_Mono
            || image_format == QImage::Format_MonoLSB
            || image_format == QImage::Format_Alpha8 )
        {
            img = img.convertToFormat( QImage::Format_Grayscale8 );
        }
        else if(    image_format >= QImage::Format_Indexed8
                 && image_format <= QImage::Format_A2RGB30_Premultiplied
                 && image_format != QImage::Format_RGB32 )
        {
            if ( img.isGrayscale() ) // image couleur avec 3 composantes identiques
            {
                img = img.convertToFormat( QImage::Format_Grayscale8 );
            }
            else
            {
                img = img.convertToFormat( QImage::Format_RGB32 );
            }
        }
        else if ( image_format == QImage::Format_Invalid )
        {
            std::cerr << "Invalid format." << std::endl;
        }

        image_format = img.format();

        imageView->displayImage( img );
        QApplication::processEvents();
        is_input_image_displayed = true;
        statusBar()->showMessage(tr("Push the left mouse button or click on the right mouse button in the window."));

        QString name = fi.fileName();
        last_directory_used = fi.absolutePath();
        setCurrentFile( fileName );

        img_width = img.width();
        img_height = img.height();
        img_size = img_width*img_height;

        positionX = img_width/2;
        positionY = img_height/2;

        phi_text->setText("ϕ(x,y,t) ∈ { -3, -1, 1, 3 }");
        iter_text->setToolTip(tr("must be strictly less than iteration_max = 5 × max(img_width,img_height) = ")+QString::number(5*std::max(img_width,img_height)));

        if( ac != nullptr )
        {
            delete ac;
            ac = nullptr;
        }

        // Paramètres d'entrée du contour actif        
        if( image_format == QImage::Format_Grayscale8 )
        {
            is_rgb1 = false;
        }
        else if( image_format == QImage::Format_RGB32 )
        {
            is_rgb1 = true;
        }
        else
        {
            std::cerr << "Conversion error or invalid format." << std::endl;
        }

        // img1 = img.constBits(); // pas possible

        QString string_lists_text;
        if( is_rgb1 )
        {
            string_lists_text = QString::number(img_width)+"×"+QString::number(img_height)+"×3";
        }
        else
        {
            string_lists_text = QString::number(img_width)+"×"+QString::number(img_height)+"×1";
        }

#ifdef Q_OS_MAC
        setWindowTitle(name+" - "+string_lists_text);
#else
        setWindowTitle(name+" - "+string_lists_text+" - Ofeli");
#endif

        // Création de img1
        if( img1 != nullptr )
        {
            delete[] img1;
            img1 = nullptr;
        }

        if( img1 == nullptr )
        {
            if( image_format == QImage::Format_Indexed8 ) // si l'image est en niveau de gris
            {
                is_rgb1 = false;

                img1 = new unsigned char[img_size];

                for( int y = 0; y < img_height; y++ )
                {
                    for( int x = 0; x < img_width; x++ )
                    {
                        img1[ find_offset(x,y) ] = *( img.scanLine(y)+x );
                    }
                }
            }
            else // si l'image est en couleur
            {
                if( img.isGrayscale() ) // si les 3 composantes sont identiques
                {
                    is_rgb1 = false;

                    img1 = new unsigned char[img_size];

                    QRgb pix;

                    for( int y = 0; y < img_height; y++ )
                    {
                        for( int x = 0; x < img_width; x++ )
                        {
                            pix = img.pixel(x,y);

                            img1[ find_offset(x,y) ] = (unsigned char)( qRed(pix) );
                        }
                    }
                }
                else
                {
                    is_rgb1 = true;

                    img1 = new unsigned char[4*img_size];

                    QRgb pix;

                    for( int y = 0; y < img_height; y++ )
                    {
                        for( int x = 0; x < img_width; x++ )
                        {
                            pix = img.pixel(x,y);

                            img1[ 4*find_offset(x,y)+2 ] = (unsigned char)( qRed(pix) );
                            img1[ 4*find_offset(x,y)+1 ] = (unsigned char)( qGreen(pix) );
                            img1[ 4*find_offset(x,y) ]   = (unsigned char)( qBlue(pix) );
                        }
                    }
                }
            }
        }

        if ( settings_window != nullptr )
        {
            settings_window->init(img1, img_width, img_height, is_rgb1, img);
            initialize_active_contour();
        }

        if( image_result_uchar != nullptr )
        {
            delete[] image_result_uchar;
            image_result_uchar = nullptr;
        }
        if( image_result_uchar == nullptr )
        {
            image_result_uchar = new unsigned char[4*img_size];
        }
        image_result = QImage(image_result_uchar, img_width, img_height, 4*img_width, QImage::Format_RGB32);

        if( is_rgb1 )
        {
            image_save_preprocess = QImage(img_width, img_height, QImage::Format_RGB32);
        }
        else
        {
            image_save_preprocess = QImage(img_width,img_height, QImage::Format_Indexed8);
        }

        const auto& config = AppSettings::instance();

        if( config.speed == SpeedModel::REGION_BASED )
        {
            if( is_rgb1 )
            {
                Cout_text->setText("Cout = (<font color=red>R<font color=black>,<font color=green>G<font color=black>,<font color=blue>B<font color=black>)");
                Cin_text->setText("Cin = (<font color=red>R<font color=black>,<font color=green>G<font color=black>,<font color=blue>B<font color=black>)");

                Cout_text->setToolTip(tr("outside average of the image in the (R,G,B) color space"));
                Cin_text->setToolTip(tr("inside average of the image in (R,G,B) color space"));
            }
            else
            {
                Cout_text->setText("Cout =");
                Cin_text->setText("Cin =");

                Cout_text->setToolTip(tr("outside average of the image"));
                Cin_text->setToolTip(tr("inside average of the image"));
            }
        }

        if( is_rgb1 )
        {
            pixel_text->setText(tr("img(x,y) = (<font color=red>R<font color=black>,<font color=green>G<font color=black>,<font color=blue>B<font color=black>)"));
            pixel_text->setToolTip(tr("pixel value at the position (x,y) in the (R,G,B) color space"));
        }
        else
        {
            pixel_text->setText(tr("img(x,y) = I"));
            pixel_text->setToolTip(tr("pixel value at the position (x,y)"));
        }

        lists_text->setText("Lin & Lout");

        disconnect(scale_spin0,SIGNAL(valueChanged(int)),this,SLOT(do_scale0(int)));

        scale_spin0->setMaximum(1000000/img_height);
        scale_spin0->setSingleStep(80000/(7*img_height));

        scale_slider0->setMaximum(160000/img_height);
        scale_slider0->setTickInterval(160000/(7*img_height));

        //try_graphics();

        connect(scale_spin0,SIGNAL(valueChanged(int)),this,SLOT(do_scale0(int)));

        if( img1 != nullptr )
        {
            has_algo_breaking = false;
            has_step_by_step = false;
            has_click_stopping = false;
            printAct->setEnabled(true);
            zoomInAct->setEnabled(true);
            zoomOutAct->setEnabled(true);
            normalSizeAct->setEnabled(true);
            startAct->setEnabled(true);
            saveAct->setEnabled(true);
        }
    }
}


/*void MainWindow::try_graphics()
{
    // Scene + View
    QGraphicsScene* scene = new QGraphicsScene(this);
    QGraphicsView* view   = new QGraphicsView(scene, this);

    view->setRenderHint(QPainter::Antialiasing, false);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    view->setDragMode(QGraphicsView::ScrollHandDrag);

    // --- Image ---
    QImage image(400, 300, QImage::Format_RGB32);
    image.fill(Qt::black);

    auto* imageItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    imageItem->setZValue(0);
    scene->addItem(imageItem);

    // --- Contour ---
    auto* ci = new ContourItem();
    scene->addItem(ci);

    // Exemple de contour (cercle discret)
    QVector<QPoint> contour;
    const QPoint center(200, 150);
    const int radius = 80;

    for (int a = 0; a < 360; ++a)
    {
        double rad = a * M_PI / 180.0;
        int x = center.x() + radius * std::cos(rad);
        int y = center.y() + radius * std::sin(rad);
        contour.append(QPoint(x, y));
    }

    ci->set_points(contour);

    // Scene size
    scene->setSceneRect(image.rect());

    view->resize(800, 600);
    setCentralWidget(view);
}*/

void MainWindow::initialize_active_contour()
{
    if( img1 != nullptr )
    {
        if( ac != nullptr )
        {
            delete ac;
            ac = nullptr;
        }

        const auto& config = AppSettings::instance();
        
        img1_filtered = settings_window->get_filtered_img_data();

        ofeli_ip::ContourData initial_tmp_ctr(config.Lout_init,
                                              config.Lin_init,
                                              img_width, img_height);

        ofeli_ip::Image8ConstView  image_grayscale(img1_filtered, img_width, img_height);
        ofeli_ip::Image32ConstView image_rgb(img1_filtered, img_width, img_height);

        if( config.speed == SpeedModel::REGION_BASED )
        {
            if( is_rgb1 )
            {
                ac = new ofeli_ip::RegionColorAc(image_rgb,
                                                 std::move(initial_tmp_ctr),
                                                 config.algo_config,
                                                 config.region_ac_config);
            }
            else
            {
                ac = new ofeli_ip::RegionAc(image_grayscale,
                                            std::move(initial_tmp_ctr),
                                            config.algo_config,
                                            config.region_ac_config);
            }
        }
        else if( config.speed == SpeedModel::EDGE_BASED )
        {
            ac = new ofeli_ip::EdgeAc(image_grayscale,
                                      std::move(initial_tmp_ctr),
                                      config.algo_config);
        }
    }
}

// Fonction appelée pour effectuer la segmentation
void MainWindow::start()
{
    disconnect( this, SIGNAL(signal_open()), this, SLOT(open()) );
    startAct->setEnabled(false);

    if( img1 != nullptr )
    {
        if( ac == nullptr || ac->get_state() == ofeli_ip::State::STOPPED || ac->get_total_iter() != 0 )
        {
            initialize_active_contour();
        }

        if( ac != nullptr )
        {
            show_phi_list_value();

            // Conditions de la boucle do-while

            if( has_algo_breaking )
            {
                has_algo_breaking = false;
                startAct->setEnabled(true);
                connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
                emit signal_open();
                return;
            }

            time_text->setText( tr("time =") );

            const auto& config = AppSettings::instance();

            if( config.speed == SpeedModel::REGION_BASED )
            {
                stackedWidget->setCurrentIndex(0);
            }
            else if( config.speed == SpeedModel::EDGE_BASED )
            {
                stackedWidget->setCurrentIndex(1);
            }

            //////////////////////////////////////////////////////////////////////
            // affiche une fois le contour actif avec l'image filtrée en dessous
            //////////////////////////////////////////////////////////////////////

            unsigned char I;
            unsigned char max = 0;
            unsigned char min = 255;

            int offset;

            if( config.speed == SpeedModel::REGION_BASED && is_rgb1 )
            {
                std::memcpy(image_result_uchar,img1_filtered,4*img_size);
            }
            else if( config.speed == SpeedModel::EDGE_BASED && config.has_histo_normaliz )
            {
                for( offset = 0; offset < img_size; offset++ )
                {
                    if( img1_filtered[offset] > max )
                    {
                        max = img1_filtered[offset];
                    }
                    else if( img1_filtered[offset] < min )
                    {
                        min = img1_filtered[offset];
                    }
                }

                for( offset = 0; offset < img_size; offset++ )
                {
                    I = (unsigned char)(255.f*float(img1_filtered[offset]-min)/float(max-min));

                    image_result_uchar[4*offset+2] = I;
                    image_result_uchar[4*offset+1] = I;
                    image_result_uchar[4*offset  ] = I;
                }
            }
            else // in case (REGION_BASED && !is_rgb1) || (EDGE_BASED && !has_histo_normaliz)
            {
                for( offset = 0; offset < img_size; offset++ )
                {
                    I = img1_filtered[offset];

                    image_result_uchar[4*offset+2] = I;
                    image_result_uchar[4*offset+1] = I;
                    image_result_uchar[4*offset  ] = I;
                }
            }

            if( config.has_gaussian_noise || config.has_salt_noise || config.has_speckle_noise || config.has_mean_filt || config.has_gaussian_filt || config.has_median_filt || config.has_aniso_diff || config.has_open_filt || config.has_close_filt || config.has_top_hat_filt || config.speed == SpeedModel::EDGE_BASED )
            {
                if( is_rgb1 )
                {
                    image_save_preprocess = image_result.copy(0,0,img_width,img_height);
                }
                else
                {
                    QVector<QRgb> table(256);
                    for( int I = 0; I < 256; I++ )
                    {
                        table[I] = qRgb(I,I,I);
                    }
                    image_save_preprocess.setColorTable(table);

                    for( int y = 0; y < img_height; y++ )
                    {
                        for( int x = 0; x < img_width; x++ )
                        {
                            *( image_save_preprocess.scanLine(y)+x ) = img1_filtered[ find_offset(x,y) ];
                        }
                    }
                }
            }

            put_displayed_active_contour();
            imageView->displayImage(image_result);
            QApplication::processEvents();
            is_input_image_displayed = false;

            if( has_step_by_step )
            {
                has_click_stopping = true;
            }
            infinite_loop();

            erase_displayed_active_contour(img1_filtered,min,max);

            float elapsed_time;
            std::clock_t start_time, stop_time;

            statusBar()->showMessage(tr("Active contour evolving."));
            time_text->setText("<font color=green>"+tr("time = _._____ s"));

            if( config.has_display_each )
            {
                // Pour calculer le temps d'exécution
                start_time = std::clock();

                //////////////////////////////////////////////////////////////////////
                // Boucle do-while, évolution de l'algorithme
                //////////////////////////////////////////////////////////////////////
                while( ac->get_state() != ofeli_ip::State::STOPPED )
                {
                    if( has_algo_breaking )
                    {
                        has_algo_breaking = false;
                        startAct->setEnabled(true);
                        connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
                        emit signal_open();
                        return;
                    }

                    if( has_step_by_step )
                    {
                        has_click_stopping = true;
                    }

                    erase_displayed_active_contour(img1_filtered,min,max);

                    ac->evolve_one_iteration();

                    put_displayed_active_contour();
                    QApplication::processEvents();

                    show_phi_list_value();

                    infinite_loop();
                }

                // Temps d'exécution de la boucle do-while
                stop_time = std::clock();
                elapsed_time = float(stop_time - start_time) / float(CLOCKS_PER_SEC);
            }
            // on affiche rien du tout pour evaluer le temps de calcul
            // if( !config.has_display_each )
            else
            {
                start_time = std::clock();
                ac->evolve();
                stop_time = std::clock();

                elapsed_time = float(stop_time - start_time) / float(CLOCKS_PER_SEC);

                put_displayed_active_contour();
                QApplication::processEvents();

                if( has_click_stopping )
                {
                    show_phi_list_value();
                }
                infinite_loop();
            }

            time_text->setText("<font color=red>"+tr("time = ")+QString::number(elapsed_time, 'g', 4)+" s");
            show_phi_list_value();
            statusBar()->clearMessage();

            statusBar()->showMessage(tr("Push the left mouse button or click on the right mouse button in the window."));
        }
    }

    has_step_by_step = false;
    startAct->setEnabled(true);

    connect( this, SIGNAL(signal_open()), this, SLOT(open()), Qt::QueuedConnection );
}

void MainWindow::put_displayed_active_contour()
{
    const auto& config = AppSettings::instance();

    draw_list_to_img( ac->get_l_out(),
                      config.color_out,
                      config.outside_combo,
                      image_result_uchar, img_width, img_height );

    draw_list_to_img( ac->get_l_in(),
                      config.color_in,
                      config.inside_combo,
                      image_result_uchar, img_width, img_height );
}

void MainWindow::erase_displayed_active_contour(const unsigned char* img, unsigned char min, unsigned char max)
{
    const auto& config = AppSettings::instance();

    int offset;
    unsigned char I;

    if( config.speed == SpeedModel::REGION_BASED && is_rgb1 )
    {
        if( config.outside_combo != ComboBoxColorIndex::NO )
        {
            erase_list_to_img( ac->get_l_out(),
                               img,
                               image_result_uchar );
        }

        if( config.inside_combo != ComboBoxColorIndex::NO )
        {
            erase_list_to_img( ac->get_l_in(),
                               img,
                               image_result_uchar );
        }
    }
    else if( config.speed == SpeedModel::EDGE_BASED && config.has_histo_normaliz )
    {
        if( config.outside_combo != ComboBoxColorIndex::NO )
        {
            for( const auto& point : ac->get_l_out() )
            {
                offset = point.get_offset();

                I = (unsigned char)(255.f*float(img[offset]-min)/float(max-min));

                image_result_uchar[4*offset+2] = I;
                image_result_uchar[4*offset+1] = I;
                image_result_uchar[4*offset  ] = I;
            }
        }

        if( config.inside_combo != ComboBoxColorIndex::NO )
        {
            for( const auto& point : ac->get_l_in() )
            {
                offset = point.get_offset();

                I = (unsigned char)(255.f*float(img[offset]-min)/float(max-min));

                image_result_uchar[4*offset] = I;
                image_result_uchar[4*offset+1] = I;
                image_result_uchar[4*offset+2] = I;
            }
        }
    }
    else // in case (REGION_BASED && !is_rgb1) || (EDGE_BASED && !has_histo_normaliz)
    {
        if( config.outside_combo != ComboBoxColorIndex::NO )
        {
            erase_list_to_img_grayscale( ac->get_l_out(),
                                         img,
                                         image_result_uchar );
        }

        if( config.inside_combo != ComboBoxColorIndex::NO )
        {
            erase_list_to_img_grayscale( ac->get_l_in(),
                                         img,
                                         image_result_uchar );
        }
    }
}

// Fonction appelée quand on clique sur save
void MainWindow::saveImage()
{
    const auto& config = AppSettings::instance();
    QFileInfo fi(fileName);
    QString fileName_save;
    QString selected_filter;

    // selection du chemin à partir d'une boîte de dialogue
    if( is_input_image_displayed )
    {
        fileName_save = QFileDialog::getSaveFileName(this,
                                                     tr("Save the image displayed by the main window"),
                                                     last_directory_used + "/" + fi.baseName(),
                                                     "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)",
                                                     &selected_filter);
    }
    else
    {
        show_phi_list_value();

        if( config.has_gaussian_noise || config.has_salt_noise || config.has_speckle_noise || config.has_mean_filt || config.has_gaussian_filt || config.has_median_filt || config.has_aniso_diff || config.has_open_filt || config.has_close_filt || config.has_top_hat_filt || config.speed == SpeedModel::EDGE_BASED )
        {
            fileName_save = QFileDialog::getSaveFileName(this,
                                                         tr("Save the image displayed by the main window"),
                                                         last_directory_used + "/" + tr("preprocessed_") + fi.baseName() + tr("_with_phi(t=")+QString::number( ac->get_total_iter() )+")",
                                                         "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)",
                                                         &selected_filter);

        }
        else
        {
            fileName_save = QFileDialog::getSaveFileName(this,
                                                         tr("Save the image displayed by the main window"),
                                                         last_directory_used + "/" + fi.baseName()+tr("_with_phi(t=") + QString::number( ac->get_total_iter() )+")",
                                                         "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)",
                                                         &selected_filter);
        }
    }

    QImage img_displayed = imageView->currentImage();

    if( !fileName_save.isEmpty() && !img_displayed.isNull() )
    {
        QFileInfo fi1(fileName_save);
        last_directory_used = fi1.absolutePath();


        selected_filter.remove(0,6);
        selected_filter.remove(4,1);

        img_displayed.save(fileName_save);

        if( fileName_save.endsWith(selected_filter) )
        {
            img_displayed.save(fileName_save);
        }
        else
        {
            img_displayed.save(fileName_save+selected_filter);
        }
    }

    if( (config.has_gaussian_noise || config.has_salt_noise || config.has_speckle_noise || config.has_mean_filt || config.has_gaussian_filt || config.has_median_filt || config.has_aniso_diff || config.has_open_filt || config.has_close_filt || config.has_top_hat_filt || config.speed == SpeedModel::EDGE_BASED) && !image_save_preprocess.isNull() )
    {
        QString selected_filter2;
        // sélection du chemin+nom de sauvegarde à partir d'une boîte de dialogue
        QString fileName_save2 = QFileDialog::getSaveFileName(this,
                                                              tr("Save the preprocessed image"),
                                                              last_directory_used + "/" + tr("preprocessed_") + fi.baseName(),
                                                              "BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)",
                                                              &selected_filter2);


        if( !fileName_save2.isEmpty() )
        {
            QFileInfo fi2(fileName_save2);
            last_directory_used = fi2.absolutePath();

            selected_filter2.remove(0,6);
            selected_filter2.remove(4,1);

            if ( fileName_save2.endsWith(selected_filter2) )
            {
                image_save_preprocess.save(fileName_save2);
            }
            else
            {
                image_save_preprocess.save(fileName_save2+selected_filter2);
            }
        }
    }
}

void MainWindow::print()
//! [5] //! [6]
{
    //Q_ASSERT(imageLabel->pixmap());
#ifndef QT_NO_PRINTER
//! [6] //! [7]
    QPrintDialog dialog(&printer, this);
//! [7] //! [8]
    if (dialog.exec()) {
        QPainter painter(&printer);
        // QRect rect = painter.viewport();
        painter.viewport();
        //QSize size = imageLabel->pixmap()->size();
        //size.scale(rect.size(), Qt::KeepAspectRatio);
        //painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        //painter.setWindow(imageLabel->pixmap()->rect());
        //painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

// Fonction appelée lors de l'ouverture de la fenêtre de configuration et valider ou refuser les changements
void MainWindow::display_settings()
{
    //settings_window->update_visu();

    // on s'assure que la fenêtre de configuration n'est pas déja affichée
    if( settings_window->isVisible() )
    {
        QMessageBox::critical(this, tr("Error"), tr("Close settings window before."));
    }
    else
    {
        if( settings_window->exec() == QDialog::Accepted )
        {
            // Ok = les variables vont prendre leur valeur en fonction de l'état des widgets de la fenêtre de configuration
            settings_window->apply_settings();
            initialize_active_contour();
        }
        else
        {
            // Annuler = les widgets vont reprendre leur état en fonction des valeurs des variables
            settings_window->cancel_settings();
        }
    }
}

void MainWindow::zoomIn()
{
    //scale_spin0->setValue( int(100.0f*imageLabel->get_zoomFactor()*1.25f) );
}

void MainWindow::zoomOut()
{
    //scale_spin0->setValue( int(100.0f*imageLabel->get_zoomFactor()*0.8f) );
}

void MainWindow::normalSize()
{
    //scale_spin0->setValue(100);
}

// Evénement clic souris dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( !startAct->isEnabled() )
        {
            if( event->button() == Qt::LeftButton )
            {
                statusBar()->showMessage(tr("Release the left mouse button to evolve the active contour."));
                has_step_by_step = false;
                has_click_stopping = true;
            }
            else if( event->button() == Qt::RightButton )
            {
                has_click_stopping = false;
                has_step_by_step = true;
            }
        }
        else
        {
            if( event->button() == Qt::LeftButton )
            {
                statusBar()->showMessage(tr("Release the left mouse button to evolve the active contour."));
                has_step_by_step = false;
                has_click_stopping = true;
                emit( startAct->trigger() );
            }
            else if( event->button() == Qt::RightButton )
            {
                has_step_by_step = true;
                emit( startAct->trigger() );
            }
        }
    }
}

// Evénement clic souris dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( event->button() == Qt::LeftButton )
        {
            statusBar()->showMessage(tr("Active contour evolving."));
            if( !startAct->isEnabled() )
            {
                has_step_by_step = false;
                has_click_stopping = false;
            }
        }
    }
}

// Evénement touche clavier enfoncé dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( !startAct->isEnabled() )
        {
            if( !event->isAutoRepeat() )
            {
                if( event->key() == Qt::Key_Space || event->key() == Qt::Key_Right || event->key() == Qt::Key_Pause )
                {
                    has_step_by_step = false;
                    has_click_stopping = true;
                }
            }

            if( event->key() == Qt::Key_Return || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down )
            {
                has_click_stopping = false;
                has_step_by_step = true;
            }
        }
        else
        {
            if( !event->isAutoRepeat() )
            {
                if( event->key() == Qt::Key_Space || event->key() == Qt::Key_Right || event->key() == Qt::Key_Pause )
                {
                    has_step_by_step = false;
                    has_click_stopping = true;
                    emit( startAct->trigger() );
                }
            }

            if( event->key() == Qt::Key_Return || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down )
            {
                has_step_by_step = true;
                emit( startAct->trigger() );
            }
        }
    }
}

// Evénement touche clavier relachée dans imageLabel (image de la fenêtre principale) permet de rentrer dans la boucle infinie ou d'en sortir pour bloquer momentanément l'algo et prendre du temps sur une étape du contour actif
void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    // si l'image est chargée
    if( img1 != nullptr )
    {
        if( !event->isAutoRepeat() )
        {
            if( event->key() == Qt::Key_Space || event->key() == Qt::Key_Right || event->key() == Qt::Key_Pause )
            {
                if( !startAct->isEnabled() )
                {
                    has_step_by_step = false;
                    has_click_stopping = false;
                }
            }
        }
    }
}

// Boucle infinie dont on peut rentrer ou sortir par un clic gauche ou droite (sur imageLabel, image de la fenêtre principale) lors de l'execution de l'algorithme et permettant de voir une étape particulière ou chaque étape, au rythme voulue
void MainWindow::infinite_loop()
{
    if( has_click_stopping )
    {
        if( has_step_by_step )
        {
            statusBar()->showMessage(tr("Click on the right mouse button to evolve the contour of one iteration."));
        }

        while( has_click_stopping )
        {
            QApplication::processEvents();
        }
    }
}

// Evénement déplacement souris au niveau de imageLabel pour pouvoir donner les infos pixels
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    const auto& config = AppSettings::instance();

    // si l'image est chargée
    if( img1 != nullptr && img1_filtered != nullptr )
    {
        /*
        positionX = int(float(img_width)
                        * float( ( (event->pos()).x() - imageLabel->get_xoffset() )
                                  / float( imageLabel->getPixWidth() )
                                  )
                        );

        positionY = int(float(img_height)
                        * float( (  (event->pos()).y() - imageLabel->get_yoffset() )
                                  / float(imageLabel->getPixHeight() )
                                  )
                        );*/

        if( positionX >= 0 &&
            positionX < img_width &&
            positionY >= 0 &&
            positionY < img_height )
        {
            QString img_str;
            if( config.has_gaussian_noise || config.has_salt_noise || config.has_speckle_noise || config.has_mean_filt || config.has_gaussian_filt || config.has_median_filt || config.has_aniso_diff || config.has_open_filt || config.has_close_filt || config.has_top_hat_filt || config.speed == SpeedModel::EDGE_BASED )
            {
                img_str = tr("img'(");
            }
            else
            {
                img_str = tr("img(");
            }

            int offset = positionX+positionY*img_width;

            if( !is_rgb1 )
            {   
                int I = img1_filtered[ offset ];
                QColor rgb(I,I,I);
                pixel_text->setText(img_str+
                                    QString::number(positionX)+
                                    ","+
                                    QString::number(positionY)+
                                    ") = "+
                                    "<font color="+
                                    rgb.name()+
                                    ">"+
                                    QString::number(I));
            }
            else
            {
                if( config.speed == SpeedModel::REGION_BASED )
                {
                    offset *= 3;

                    QColor rgb( img1_filtered[ offset ],
                                img1_filtered[ offset+1 ],
                                img1_filtered[ offset+2 ]);

                    pixel_text->setText(img_str+
                                        QString::number(positionX)+
                                        ","+QString::number(positionY)+
                                        ") = "+
                                        "<font color="+
                                        rgb.name()+">"+
                                        "("+QString::number( rgb.red() )+
                                        ","+QString::number( rgb.green() )+
                                        ","+QString::number( rgb.blue() )+
                                        ")");
                }
                else if( config.speed == SpeedModel::EDGE_BASED )
                {
                    int I = img1_filtered[ offset ];
                    QColor rgb(I,I,I);
                    pixel_text->setText(img_str+
                                        QString::number(positionX)+
                                        ","+
                                        QString::number(positionY)+
                                        ") = "+
                                        "<font color="+
                                        rgb.name()+
                                        ">"+
                                        QString::number(I));
                }
            }
            show_phi_list_value();
        }
    }
}

void MainWindow::show_phi_list_value()
{
    auto& config = AppSettings::instance();

    QColor RGBout, RGBin;
    int Cout1, Cin1;

    QColor RGBout_list( get_QRgb(config.color_out) );
    QColor RGBin_list( get_QRgb(config.color_in) );

    QString Cout_str = QString("<font color="+RGBout_list.name()+">"+"Cout"+"<font color=black>"+" = ");
    QString Cin_str = QString("<font color="+RGBin_list.name()+">"+"Cin"+"<font color=black>"+" = ");

    unsigned char otsu_threshold;

    if( ac != nullptr )
    {
        QString green_str = "<font color=green>";
        QString red_str = "<font color=red>";
        QString color_str;

        if( ac->get_state() == ofeli_ip::State::CYCLE_1 )
        {
            state_text->setText(green_str+tr("state = CYCLE_1"));
        }
        else if( ac->get_state() == ofeli_ip::State::CYCLE_2 )
        {
            state_text->setText(green_str+tr("state = CYCLE_2"));
        }
        else if( ac->get_state() == ofeli_ip::State::LAST_CYCLE_2 )
        {
            state_text->setText(green_str+tr("state = LAST_CYCLE_2"));
        }
        else if( ac->get_state() == ofeli_ip::State::STOPPED )
        {
            state_text->setText(red_str+tr("state = STOPPED"));
        }


        if( ac->get_stopping_status() == ofeli_ip::StoppingStatus::MAX_ITERATION )
        {
            color_str = red_str;
        }
        else
        {
            color_str = green_str;
        }

        iter_text->setText(color_str+tr("iteration t = ")+QString::number( ac->get_total_iter() ));

        if( ac->get_stopping_status() == ofeli_ip::StoppingStatus::LISTS_STOPPED )
        {
            changes_text->setText(red_str+tr("lists changes = false"));
        }
        else
        {
            changes_text->setText(green_str+tr("lists changes = true"));
        }

        if( ac->get_stopping_status() == ofeli_ip::StoppingStatus::HAUSDORFF )
        {
            color_str = red_str;
        }
        else
        {
            color_str = green_str;
        }

        quantile_text->setText(color_str+tr("80th Hausdorff quantile = ")+QString::number( ac->get_hausdorff_quantile(), 'g', 3 )+" %");
        centroids_text->setText(color_str+tr("centroid evolution = ")+QString::number( ac->get_centroids_distance(), 'g', 3 )+" %");

        if( config.speed == SpeedModel::REGION_BASED )
        {
            if( !is_rgb1 )
            {
                // pour l'affichage
                if( dynamic_cast<ofeli_ip::RegionAc*>(ac) != nullptr )
                {
                    Cout1 = dynamic_cast<ofeli_ip::RegionAc*>(ac)->get_Cout();
                    Cin1  = dynamic_cast<ofeli_ip::RegionAc*>(ac)->get_Cin();

                    RGBout = QColor(Cout1, Cout1, Cout1);
                    RGBin  = QColor(Cin1, Cin1, Cin1);

                    Cout_text->setText(Cout_str+"<font color="+RGBout.name()+">"+QString::number(Cout1));
                    Cin_text->setText(Cin_str+"<font color="+RGBin.name()+">"+QString::number(Cin1));
                }
            }
            else
            {
                if( dynamic_cast<ofeli_ip::RegionColorAc*>(ac) != nullptr )
                {
                    const ofeli_ip::Rgb_uc& Cout_RGB = dynamic_cast<ofeli_ip::RegionColorAc*>(ac)->get_Cout();
                    const ofeli_ip::Rgb_uc& Cin_RGB  = dynamic_cast<ofeli_ip::RegionColorAc*>(ac)->get_Cin();

                    RGBout = QColor( Cout_RGB.red,
                                     Cout_RGB.green,
                                     Cout_RGB.blue );

                    RGBin = QColor( Cin_RGB.red,
                                    Cin_RGB.green,
                                    Cin_RGB.blue );

                    Cout_text->setText(Cout_str+"<font color="+RGBout.name()+">"+"("+QString::number(Cout_RGB.red)+","+QString::number(Cout_RGB.green)+","+QString::number(Cout_RGB.blue)+")");
                    Cin_text->setText(Cin_str+"<font color="+RGBin.name()+">"+"("+QString::number(Cin_RGB.red)+","+QString::number(Cin_RGB.green)+","+QString::number(Cin_RGB.blue)+")");
                }
            }
        }
        else if( config.speed == SpeedModel::EDGE_BASED )
        {
            if( dynamic_cast<ofeli_ip::EdgeAc*>(ac) != nullptr )
            {
                // pour l'affichage
                otsu_threshold = dynamic_cast<ofeli_ip::EdgeAc*>(ac)->get_threshold();
                threshold_text->setText(tr("Otsu's threshold of gradient = ")+QString::number(otsu_threshold));
            }
        }

        const ofeli_ip::Matrix<ofeli_ip::PhiValue>& phi = ac->get_phi();
        if( !is_input_image_displayed &&
            !phi.is_null() &&
            positionX >= 0 &&
            positionY >= 0 &&
            positionX < img_width &&
            positionY < img_height )
        {
            QColor RGBout( get_QRgb(config.color_out) );
            QColor RGBin( get_QRgb(config.color_in) );

            if( phi(positionX,positionY) == ofeli_ip::PhiValue::OUTSIDE_REGION )
            {
                phi_text->setText("ϕ(x,y) = <font color="+RGBout.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∉ <font color="+RGBout.name()+">"+"Lout"+"<font color=black>"+" & ∉ "+"<font color="+RGBin.name()+">"+"Lin");
            }
            else if( phi(positionX,positionY) == ofeli_ip::PhiValue::INSIDE_REGION )
            {
                phi_text->setText("ϕ(x,y) = <font color="+RGBin.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∉ <font color="+RGBin.name()+">"+"Lin"+"<font color=black>"+" & ∉ "+"<font color="+RGBout.name()+">"+"Lout");
            }
            else if( phi(positionX,positionY) == ofeli_ip::PhiValue::EXTERIOR_BOUNDARY )
            {
                phi_text->setText("ϕ(x,y) = <font color="+RGBout.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∈ <font color="+RGBout.name()+">"+"Lout");
            }
            else if( phi(positionX,positionY) == ofeli_ip::PhiValue::INTERIOR_BOUNDARY )
            {
                phi_text->setText("ϕ(x,y) = <font color="+RGBin.name()+">"+QString::number(phi(positionX,positionY)));
                lists_text->setText("(x,y) ∈ <font color="+RGBin.name()+">"+"Lin");
            }
        }
    }
}

// Pour arrêter manuellement l'algorithme, s'il y a un problème au niveau des conditions d'arrêts ou du temps de calcul sur les grandes images
void MainWindow::stop()
{
    has_click_stopping = false;
    has_algo_breaking = true;
}

// Filtre des événements pour avoir le tracking au niveau du widget image de la fenêtre principale et de la fenêtre de parametres et pour ne pas avoir le tracking/la position au niveau de l'ensemble de chaque fenêtre
//bool MainWindow::eventFilter(QObject* object, QEvent* event)
//{
    /*
    if( object == imageLabel )
    {
        if( event->type() == QEvent::DragEnter )
        {
            QDragEnterEvent* drag = static_cast<QDragEnterEvent*>(event);
            dragEnterEvent(drag);
        }
        else if( event->type() == QEvent::DragMove )
        {
            QDragMoveEvent* drag = static_cast<QDragMoveEvent*>(event);
            dragMoveEvent(drag);
        }
        else if( event->type() == QEvent::Drop )
        {
            QDropEvent* drag = static_cast<QDropEvent*>(event);
            dropEvent(drag);
        }
        else if( event->type() == QEvent::DragLeave )
        {
            QDragLeaveEvent* drag = static_cast<QDragLeaveEvent*>(event);
            dragLeaveEvent(drag);
        }
        else if( event->type() == QEvent::MouseMove )
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouseMoveEvent(mouseEvent);

            if( positionX >= 0 &&
                positionX < img_width &&
                positionY >= 0 &&
                positionY < img_height )
            {
                setCursor(Qt::CrossCursor);
            }
            else
            {
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    else if( object == scale_spin0 || object == scale_slider0 )
    {
        if( event->type() == QEvent::MouseButtonPress )
        {
            if( img1 != nullptr )
            {
                //imageLabel->set_has_text(false);
                //imageLabel->setBackgroundRole(QPalette::Dark);
            }
            positionX = img_width/2;
            positionY = img_height/2;
        }
    }

    return false;*/
//}

// Fonction appelée pour le changement d'échelle dans la fenêtre viewer
void MainWindow::do_scale0(int value)
{
    if( img1 != nullptr )
    {
        //imageLabel->setZoomFactor(float(value)/100.0);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if( !settings_window->isVisible() )
    {
        QString text(tr("<drop image>"));
        //imageLabel->set_text(text);
        //imageLabel->setBackgroundRole(QPalette::Highlight);
        //imageLabel->set_has_text(true);

        event->acceptProposedAction();
        emit changed(event->mimeData());
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    if( !settings_window->isVisible() )
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if( !settings_window->isVisible() )
    {
        const QMimeData* mimeData = event->mimeData();

        if( mimeData->hasUrls() )
        {
            QList<QUrl> urlList = mimeData->urls();
            fileName = urlList.first().toLocalFile();
        }
        //imageLabel->setBackgroundRole(QPalette::Dark);
        emit signal_open();
        event->acceptProposedAction();
    }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    if( !settings_window->isVisible() )
    {
        QString text(tr("<drag image>"));
        //imageLabel->set_text(text);
        //imageLabel->setBackgroundRole(QPalette::Dark);
        //imageLabel->set_has_text(true);
        emit changed();
        event->accept();
    }
}

void MainWindow::adjustVerticalScroll(int min, int max)
{
    if( img_height != 0 )
    {
        //scrollArea->verticalScrollBar()->setValue( (max-min)*positionY/img_height );
        //scrollArea->verticalScrollBar()->setValue( scrollArea->verticalScrollBar()->value() );
        //std ::cout << scrollArea->verticalScrollBar()->value() << std::endl;
        //std ::cout << min << std::endl;
        //std ::cout << max << std::endl;
    }
}

void MainWindow::adjustHorizontalScroll(int min, int max)
{
    if( img_width != 0 )
    {
        //scrollArea->horizontalScrollBar()->setValue( (max-min)*positionX/img_width );
    }
}

void MainWindow::language()
{
    if( language_window->exec() == QDialog::Accepted )
    {
        language_window->apply_setting();
    }
    else
    {
        language_window->cancel_setting();
    }
}

void MainWindow::doc()
{
    QDesktopServices::openUrl( QUrl("http://ofeli.googlecode.com/svn/doc/index.html", QUrl::TolerantMode) );
}

void MainWindow::setCurrentFile(const QString &fileName1)
{
    curFile = fileName1;
    //setWindowFilePath(curFile);

    QSettings settings;
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();
    files.removeAll(fileName1);
    files.prepend(fileName1);
    while( files.size() > MaxRecentFiles )
    {
        files.removeLast();
    }

    settings.setValue("Main/Name/recentFileList", files);

    foreach( QWidget* widget, QApplication::topLevelWidgets() )
    {
        MainWindow* mainWindow = qobject_cast<MainWindow*>(widget);
        if( mainWindow != nullptr )
        {
            mainWindow->updateRecentFileActions();
        }
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for( int i = 0; i < numRecentFiles; ++i )
    {
        QString text = tr("&%1").arg( strippedName(files[i]) );
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
        recentFileActs[i]->setStatusTip(files[i]);
    }

    for( int j = numRecentFiles; j < MaxRecentFiles; ++j )
    {
        recentFileActs[j]->setVisible(false);
    }

    separatorAct->setVisible(numRecentFiles > 0);

    if( files.isEmpty() )
    {
        deleteAct->setVisible(false);
    }
    else
    {
        deleteAct->setVisible(true);
    }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>( sender() );
    if( action != nullptr )
    {
        fileName = action->data().toString();
    }

    emit signal_open();
}

void MainWindow::deleteList()
{
    QStringList files;
    files.clear();

    QSettings settings;
    settings.setValue("Main/Name/recentFileList", files);

    updateRecentFileActions();
}

void MainWindow::wheel_zoom(int val, ScrollAreaWidget* obj)
{
    //if( obj == scrollArea && img1 != nullptr )
    //{
        //imageLabel->set_has_text(false);
        //imageLabel->setBackgroundRole(QPalette::Dark);

        //float value = 0.002f*float( val ) + imageLabel->get_zoomFactor();

        //if( value < 32.0f/float( imageLabel->get_qimage().width() ) )
        //{
         //   value = 32.0f/float( imageLabel->get_qimage().width() );
        //}

        //scale_spin0->setValue( int(100.0f*value) );
    //}
}

void MainWindow::set_zoom_factor(int val)
{
    scale_spin0->setValue(val);
}

int MainWindow::get_zoom_factor() const
{
    return scale_spin0->value();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    auto& config = AppSettings::instance();
    QSettings settings;

    settings.setValue( "Main/Window/geometry", saveGeometry() );
    settings.setValue( "Main/Name/last_directory_used", last_directory_used);

    config.save();

    QMainWindow::closeEvent(event);
}

void MainWindow::updateCameraAction()
{
    auto cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty())
    {
        cameraAct->setEnabled(false);
        cameraAct->setText(tr("Camera"));
    }
    else
    {
        cameraAct->setEnabled(true);
        cameraAct->setText(tr("Camera"));
    }
}

void MainWindow::onStartCameraActionTriggered()
{
    auto cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty()) {
        QMessageBox::information(this, tr("Information"), tr("No camera available."));
        return;
    } else if (cameras.size() == 1) {
        camera_window->show();
    } else {
        camera_window->show();
    }
}

}
