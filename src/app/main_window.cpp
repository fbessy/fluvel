#include "main_window.hpp"

#include "image_view_base.hpp"
#include "image_pipeline_controller.hpp"
#include "active_contour_worker.hpp"

#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>

namespace ofeli_gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupActions();
    setupConnections();

    updateRecentFileActions();
    updateCameraAction();

    QSettings settings;
    last_directory_used = settings.value("Main/Name/last_directory_used",QDir().homePath()).toString();
}

void MainWindow::setupUi()
{
    setWindowTitle( tr("Ofeli") );
    setWindowIcon( QIcon(":/icons/app/Ofeli.svg") );

    QSettings settings;

    const auto geo = settings.value("Main/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    camera_window = new CameraWindow(this);
    evaluation_window = new EvaluationWindow(this);
    settings_window = new SettingsWindow(this);
    about_window = new AboutWindow(this);
    language_window = new LanguageWindow(this);

    startButton = new QPushButton( tr("Start") );
    pauseButton = new QPushButton( tr("Pause") );
    stepButton = new QPushButton( tr("Setp") );

    // Widget central
    QWidget* central = new QWidget(this);

    // Layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Control bar ---
    QWidget* controlBar = new QWidget(central);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(6);

    controlLayout->addWidget(startButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(stepButton);
    controlLayout->addStretch();
    // --- Image view ---

    imageView = new ImageViewBase(central);
    imageView->setMaxDisplayFps(60);

    // Assemblage
    mainLayout->addWidget(controlBar);
    mainLayout->addWidget(imageView, 1); // stretch = important

    setCentralWidget(central);
}

void MainWindow::setupActions()
{
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


    deleteAct = new QAction(tr("Clear list"), this);
    deleteAct->setStatusTip(tr("Clean the recent files list."));

    deleteAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::EditClear,
                                        QIcon(":/icons/toolbar/edit-clear-list.svg")) );

    saveAct = new QAction(tr("S&ave..."), this);
    saveAct->setShortcut(tr("Ctrl+A"));
    saveAct->setEnabled(false);
    saveAct->setStatusTip(tr("Save the displayed and preprocessed images."));

    saveAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs,
                                      style->standardIcon(QStyle::SP_DialogSaveButton)) );

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+X"));

    exitAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit,
                                      style->standardIcon(QStyle::SP_TitleBarCloseButton)) );

    cameraAct = new QAction(tr("Camera"), this);
    cameraAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::CameraWeb) );
    cameraAct->setEnabled(false);

    mediaDevices = new QMediaDevices(this);

    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);

        recentFileActs[i]->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpenRecent,
                                                    QIcon(":/icons/toolbar/document-open-recent.svg")) );
    }

    evaluateAct = new QAction(tr("E&valuate"), this);
    evaluateAct->setStatusTip(tr("Compute the modified Hausdorff distance."));
    evaluateAct->setShortcut(tr("Ctrl+V"));
    evaluateAct->setEnabled(true);

    evaluateAct->setIcon( QIcon::fromTheme("accessories-calculator",
                                          QIcon(":/icons/toolbar/insert-horizontal-rule.svg")) );

    settingsAct = new QAction(tr("S&ettings"), this);
    settingsAct->setStatusTip(tr("Image preprocessing and active contour initialization."));
    settingsAct->setShortcut(tr("Ctrl+E"));
    settingsAct->setEnabled(true);

    settingsAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties,
                                          QIcon(":/icons/toolbar/configure.svg")) );


    menuBar()->addSeparator();

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Information, license and home page."));

    aboutAct->setIcon( QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout,
                                       style->standardIcon(QStyle::QStyle::SP_MessageBoxInformation)) );


    languageAct = new QAction(tr("&Language"), this);
    languageAct->setStatusTip(tr("Choose the application language."));

    languageAct->setIcon( QIcon::fromTheme("preferences-desktop-locale",
                                          QIcon(":/icons/toolbar/preferences-desktop-locale.svg")) );


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
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    windowMenu = new QMenu(tr("&Window"), this);
    windowMenu->addAction(cameraAct);
    windowMenu->addAction(evaluateAct);
    windowMenu->addAction(settingsAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(languageAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(windowMenu);
    menuBar()->addMenu(helpMenu);

    imageController = new ImagePipelineController(this);
    acWorker = new ActiveContourWorker;
}

void MainWindow::setupConnections()
{
    nameFilters << "*.bmp"
                << "*.gif"
                << "*.jpg" << "*.jpeg" << "*.mng"
                << "*.pbm" << "*.png" << "*.pgm"
                << "*.ppm" << "*.svg" << "*.svgz"
                << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    nameFilters.removeDuplicates();

    connect(openAct, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Image"),
                                                        last_directory_used,
                                                        tr("Image Files (%1)").arg(nameFilters.join(" ")));
        if ( !fileName.isEmpty() ) {
            last_directory_used = QFileInfo(fileName).absolutePath();
            emit fileSelected(fileName);
        }
    });

    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        connect(recentFileActs[i], &QAction::triggered, this, [this]() {
            QAction* action = qobject_cast<QAction*>( sender() );
            QString fileName;
            if( action != nullptr )
            {
                fileName = action->data().toString();
            }
            if ( !fileName.isEmpty() ) {
                emit fileSelected(fileName);
            }
        });
    }
    connect(this, &MainWindow::fileSelected, imageController, &ImagePipelineController::loadImage);

    //connect(deleteAct, &QAction::triggered, this, &MainWindow::deleteList);
    //connect(saveAct, &QAction::triggered, this, &MainWindow::saveImage);
    connect(exitAct, &QAction::triggered, this, &MainWindow::close);

    connect(cameraAct, &QAction::triggered, this, &MainWindow::onStartCameraActionTriggered);
    connect(mediaDevices, &QMediaDevices::videoInputsChanged, this, &MainWindow::updateCameraAction);
    connect(evaluateAct, &QAction::triggered, evaluation_window, &EvaluationWindow::show);
    connect(settingsAct, &QAction::triggered, settings_window, &SettingsWindow::show);

    connect(aboutAct, &QAction::triggered, about_window, &AboutWindow::show);
    connect(languageAct, &QAction::triggered, language_window, &LanguageWindow::show);


    connect(imageController, &ImagePipelineController::imageReady,
            imageView,       &ImageViewBase::displayImage);

    connect(imageController, &ImagePipelineController::contourReady,
            acWorker,        &ActiveContourWorker::setImage);

    connect(acWorker,  &ActiveContourWorker::resultReady,
            imageView, &ImageViewBase::displayImage);

    connect(startButton,   &QPushButton::clicked,
            acWorker,      &ActiveContourWorker::start);

    connect(pauseButton,  &QPushButton::clicked,
            acWorker,     &ActiveContourWorker::restart);

    connect(stepButton,   &QPushButton::clicked,
            acWorker,     &ActiveContourWorker::stop);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
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

void MainWindow::updateCameraAction()
{
    auto cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty())
    {
        cameraAct->setEnabled(false);
    }
    else
    {
        cameraAct->setEnabled(true);
    }
}

void MainWindow::onStartCameraActionTriggered()
{
    auto cameras = QMediaDevices::videoInputs();

    if( cameras.isEmpty() )
    {
        QMessageBox::information(this, tr("Information"), tr("No camera available."));
    }
    else
    {
        camera_window->show();
    }
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

}
