#include "image_window.hpp"

#include "image_view.hpp"
#include "image_controller.hpp"
#include "active_contour_worker.hpp"
#include "algo_info_overlay.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "fullscreen_behavior.hpp"
#include "autofit_behavior.hpp"
#include "pixel_info_behavior.hpp"
#include "icon_loader.hpp"

#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>

namespace ofeli_app {

ImageWindow::ImageWindow(QWidget* parent)
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

void ImageWindow::setupUi()
{
    updateWindowTitle();
    setWindowIcon( QIcon(":/icons/app/Ofeli.svg") );

    QSettings settings;

    const auto geo = settings.value("Main/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    startResumeIcon = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart,
                                   QStyle::SP_MediaPlay,
                                   ":/icons/toolbar/media-playback-start-symbolic.svg");

    restartIcon = il::loadIcon(QIcon::ThemeIcon::MediaPlaylistRepeat,
                               QStyle::SP_BrowserReload,
                               ":/icons/toolbar/media-playlist-repeat-symbolic.svg");

    pauseIcon = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackPause,
                             QStyle::SP_MediaPause,
                             ":/icons/toolbar/media-playback-pause-symbolic.svg");

    restartButton = new QPushButton( tr("Start") );
    restartButton->setToolTip(tr("Run the active contour."));
    restartButton->setIcon( startResumeIcon );

    togglePauseButton = new QPushButton( tr("Resume") );
    togglePauseButton->setToolTip(tr("Resume the active contour execution."));
    togglePauseButton->setIcon( startResumeIcon );

    stepButton = new QPushButton( tr("Step") );
    stepButton->setToolTip(tr("Advance the active contour by one iteration."));

    QIcon stepIcon = il::loadIcon(QIcon::ThemeIcon::GoNext,
                                  QStyle::SP_ArrowRight,
                                  ":/icons/toolbar/go-next-symbolic.svg");

    stepButton->setIcon( stepIcon );

    stepButton->setAutoRepeat(true);
    stepButton->setAutoRepeatDelay(300);
    stepButton->setAutoRepeatInterval(100);

    convergeButton = new QPushButton( tr("Converge") );
    convergeButton->setToolTip(tr("Run until completion without displaying intermediate steps."));

    QIcon convergeIcon = il::loadIcon(QIcon::ThemeIcon::MediaSeekForward,
                                      QStyle::SP_MediaSeekForward,
                                      ":/icons/toolbar/media-seek-forward-symbolic.svg");

    convergeButton->setIcon( convergeIcon );

    restartButton->setEnabled(false);
    togglePauseButton->setEnabled(false);
    stepButton->setEnabled(false);
    convergeButton->setEnabled(false);


    rightPanelToggle = new QPushButton;
    rightPanelToggle->setCheckable(true);
    rightPanelToggle->setChecked(true);
    rightPanelToggle->setFocusPolicy(Qt::NoFocus);
    rightPanelToggle->setToolTip(tr("Right panel is visible."));

    rightPanelToggle->setIcon(QIcon(":/icons/toolbar/right_panel_on.svg"));

    settingsButton = new QPushButton;
    settingsButton->setToolTip(tr("Camera session settings"));
    settingsButton->setFlat(true);
    settingsButton->setFocusPolicy(Qt::NoFocus);

    settingsIcon = il::loadIcon("configure",
                                ":/icons/toolbar/configure-symbolic.svg");

    settingsButton->setIcon( settingsIcon );

    // Widget central
    QWidget* central = new QWidget(this);

    // Layout principal vertical
    QVBoxLayout* vLayout = new QVBoxLayout(central);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    // --- Control bar ---
    QWidget* controlBar = new QWidget(central);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(8, 4, 8, 4);
    controlLayout->setSpacing(6);

    controlLayout->addWidget(restartButton);
    controlLayout->addWidget(togglePauseButton);
    controlLayout->addWidget(stepButton);
    controlLayout->addWidget(convergeButton);
    controlLayout->addStretch();
    controlLayout->addWidget(rightPanelToggle);
    controlLayout->addSpacerItem(
        new QSpacerItem(12, 0, QSizePolicy::Fixed, QSizePolicy::Minimum)
        );
    controlLayout->addWidget(settingsButton);

    // --- Image view ---
    imageView = new ImageView(central);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    imageView->setInteraction(interaction.release());

    imageOverlay = new AlgoInfoOverlay(imageView->viewport());
    imageOverlay->raise();

    // --- Display bar (à droite) ---
    displayBar = new DisplaySettingsWidget(central, Session::Image);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(imageView, 1);  // prend tout l'espace
    contentLayout->addWidget(displayBar, 0); // largeur naturelle

    // Assemblage global
    vLayout->addWidget(controlBar);
    vLayout->addLayout(contentLayout);

    setCentralWidget(central);
}

void ImageWindow::setupActions()
{
    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Actions                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    imageSessionAct = new QAction(tr("&Image"), this);
    imageSessionAct->setShortcut(tr("Ctrl+I"));

    QIcon imageIcon = il::loadIcon("image-x-generic-symbolic",
                                   ":/icons/toolbar/view-preview-symbolic.svg");

    imageSessionAct->setIcon( imageIcon );
    imageSessionAct->setCheckable(true);
    imageSessionAct->setChecked(true);
    imageSessionAct->setEnabled(true);

    cameraSessionAct = new QAction(tr("Came&ra"), this);
    cameraSessionAct->setShortcut(tr("Ctrl+R"));

    QIcon cameraIcon = il::loadIcon(QIcon::ThemeIcon::CameraWeb,
                                    ":/icons/toolbar/camera-web-symbolic.svg");

    cameraSessionAct->setIcon( cameraIcon );

    cameraSessionAct->setCheckable(true);
    cameraSessionAct->setChecked(false);
    cameraSessionAct->setEnabled(false);

    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcut(QKeySequence::Quit);

    QIcon quitIcon = il::loadIcon(QIcon::ThemeIcon::ApplicationExit,
                                  QStyle::SP_TitleBarCloseButton,
                                  ":/icons/toolbar/application-exit-symbolic.svg");

    quitAct->setIcon( quitIcon );

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an image file (*.png, *.bmp, *.jpg, *.jpeg, *.tiff, *.tif, *.gif, *.pbm, *.pgm, *.ppm, *.svg, *.svgz, *.mng, *.xbm, *.xpm)."));

    QIcon openIcon = il::loadIcon(QIcon::ThemeIcon::DocumentOpen,
                                  QStyle::SP_DirOpenIcon,
                                  ":/icons/toolbar/document-open-symbolic.svg");

    openAct->setIcon( openIcon );


    deleteAct = new QAction(tr("Clear list"), this);
    deleteAct->setStatusTip(tr("Clean the recent files list."));

    QIcon deleteIcon = il::loadIcon(QIcon::ThemeIcon::EditClear,
                                    QStyle::SP_LineEditClearButton,
                                    ":/icons/toolbar/edit-clear-history.svg");

    deleteAct->setIcon( deleteIcon );

    saveAct = new QAction(tr("&Save..."), this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the displayed image."));

    QIcon saveIcon = il::loadIcon(QIcon::ThemeIcon::DocumentSaveAs,
                                  QStyle::SP_DialogSaveButton,
                                  ":/icons/toolbar/document-save-as-symbolic.svg");

    saveAct->setIcon( saveIcon );


    mediaDevices = new QMediaDevices(this);

    QIcon recentIcon = il::loadIcon(QIcon::ThemeIcon::DocumentOpenRecent,
                                    QStyle::SP_DirOpenIcon,
                                    ":/icons/toolbar/document-open-recent-symbolic.svg");

    for( int i = 0; i < MaxRecentFiles; ++i )
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);

        recentFileActs[i]->setIcon( recentIcon );
    }

    analysisAct = new QAction(tr("&Analysis"), this);
    analysisAct->setStatusTip(tr("Compute the Hausdorff distance."));
    analysisAct->setShortcut(tr("Ctrl+A"));

    analysisAct->setIcon( QIcon(":/icons/toolbar/measure-symbolic.svg") );

    settingsAct = new QAction(tr("&Settings"), this);
    settingsAct->setShortcut(QKeySequence::Preferences);
    settingsAct->setStatusTip(tr("Image preprocessing and active contour initialization."));
    settingsAct->setEnabled(true);

    settingsAct->setIcon( settingsIcon );

    menuBar()->addSeparator();

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Information, license and home page."));

    QIcon aboutIcon = il::loadIcon(QIcon::ThemeIcon::HelpAbout,
                                   QStyle::SP_MessageBoxInformation,
                                   ":/icons/toolbar/help-about-symbolic.svg");

    aboutAct->setIcon( aboutIcon );


    languageAct = new QAction(tr("&Language"), this);
    languageAct->setStatusTip(tr("Choose the application language."));

    QIcon languageIcon = il::loadIcon("preferences-desktop-locale",
                                      ":/icons/toolbar/preferences-desktop-locale.svg");

    languageAct->setIcon( languageIcon );


    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Menus                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    sessionMenu = new QMenu(tr("&Session"), this);
    sessionMenu->addAction(imageSessionAct);
    sessionMenu->addAction(cameraSessionAct);
    sessionMenu->addSeparator();
    sessionMenu->addAction(quitAct);

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

    segmentationMenu = new QMenu(tr("&Segmentation"), this);
    segmentationMenu->addAction(analysisAct);
    segmentationMenu->addAction(settingsAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(languageAct);

    menuBar()->addMenu(sessionMenu);
    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(segmentationMenu);
    menuBar()->addMenu(helpMenu);

    imageController = new ImageController(this);

    phiEditor = std::make_unique<PhiEditor>();
    phiViewModel = std::make_unique<PhiViewModel>(phiEditor.get());

    settings_window = new SettingsWindow(this, phiEditor.get(),
                                               phiViewModel.get());

    camera_window = new CameraWindow;
    evaluation_window = new AnalysisWindow(this);
    about_window = new AboutWindow(this);
    language_window = new LanguageWindow(this);
}

void ImageWindow::setupConnections()
{
    connect(this, &ImageWindow::fileSelected,
            this, &ImageWindow::onFileSelected);

    connect(imageController, &ImageController::displayedImageReady,
            this,            &ImageWindow::onDisplayedImageReady);


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
    connect(this,            &ImageWindow::fileSelected,
            imageController, &ImageController::loadImage);

    connect(deleteAct, &QAction::triggered,
            this,      &ImageWindow::deleteList);

    connect(saveAct, &QAction::triggered,
            this,    &ImageWindow::saveDisplayed);

    connect(quitAct, &QAction::triggered,
            this,    &ImageWindow::close);

    connect(imageSessionAct, &QAction::triggered, this, [this]() {
        imageSessionAct->setChecked(true);
    });

    connect(cameraSessionAct, &QAction::triggered,
            this,             &ImageWindow::onStartCameraActionTriggered);

    connect(camera_window, &CameraWindow::cameraWindowClosed,
            this,          &ImageWindow::onCameraWindowClosed);
    connect(camera_window, &CameraWindow::cameraWindowShown,
            this,          &ImageWindow::onCameraWindowShown);


    connect(mediaDevices,      &QMediaDevices::videoInputsChanged,
            this,              &ImageWindow::updateCameraAction);

    connect(analysisAct,       &QAction::triggered,
            evaluation_window, &AnalysisWindow::show);

    connect(settingsAct,     &QAction::triggered,
            settings_window, &SettingsWindow::show);

    connect(aboutAct,        &QAction::triggered,
            about_window,    &AboutWindow::show);

    connect(languageAct,     &QAction::triggered,
            language_window, &LanguageWindow::show);

    imageView->applyDownscaleConfig( AppSettings::instance().imgConfig.compute.downscale );
    imageView->applyDisplayConfig( AppSettings::instance().imgConfig.display );

    connect(&AppSettings::instance(), &ApplicationSettings::imgSettingsChanged,
            this, [this](const ImageSessionSettings& conf) {
                imageView->applyDownscaleConfig( conf.compute.downscale );
            });

    connect(&AppSettings::instance(),
            &ApplicationSettings::imgDisplaySettingsChanged,
            imageView,
            &ImageView::applyDisplayConfig);

    connect(imageController,
            &ImageController::clearOverlaysRequested,
            imageView,
            &ImageView::clearOverlays);

    connect(imageController,
            &ImageController::displayedImageReady,
            imageView,
            &ImageView::setImage);

    connect(imageController,
            &ImageController::contourUpdated,
            imageView,
            &ImageView::setContour,
            Qt::QueuedConnection);

    QTimer* m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(30); // ~33 FPS
    connect(m_statsTimer, &QTimer::timeout,
            this, &ImageWindow::refreshAlgoOverlay);
    m_statsTimer->start();

    connect(restartButton,      &QPushButton::clicked,
            imageController,    &ImageController::restart);

    connect(togglePauseButton,  &QPushButton::clicked,
            imageController,    &ImageController::togglePause);

    connect(stepButton,         &QPushButton::clicked,
            imageController,    &ImageController::step);

    connect(convergeButton,     &QPushButton::clicked,
            imageController,    &ImageController::converge);

    connect(rightPanelToggle, &QPushButton::toggled,
            this, [this](bool checked)
            {
                if ( checked )
                {
                    rightPanelToggle->setIcon(QIcon(":/icons/toolbar/right_panel_on.svg"));
                    rightPanelToggle->setToolTip(tr("Right panel is visible."));
                }
                else
                {
                    rightPanelToggle->setIcon(QIcon(":/icons/toolbar/right_panel_off.svg"));
                    rightPanelToggle->setToolTip(tr("Right panel is hidden."));
                }

                displayBar->setVisible(checked);
            });

    connect(settingsButton,     &QPushButton::clicked,
            settings_window,    &SettingsWindow::show);

    connect(imageController,    &ImageController::stateChanged,
            this,               &ImageWindow::onStateChanged);

    connect(imageController,    &ImageController::imageReadyWithoutResize,
            phiViewModel.get(), &PhiViewModel::setBackgroundWithUpdate);

    connect(imageController,
            &ImageController::imageReadyWithResize,
            phiEditor.get(),
            [this](const QImage &img)
            {
                phiViewModel->setBackground(img);
                phiEditor->onImageSizeReady(img.width(), img.height());
            });
}

QString ImageWindow::strippedName(const QString &fullFilename)
{
    return QFileInfo(fullFilename).fileName();
}

void ImageWindow::setCurrentFile(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while( files.size() > MaxRecentFiles )
    {
        files.removeLast();
    }

    settings.setValue("Main/Name/recentFileList", files);
    updateRecentFileActions();
}

void ImageWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();

    qsizetype numRecentFiles = qMin(files.size(),
                                    MaxRecentFiles);

    for( qsizetype i = 0; i < numRecentFiles; ++i )
    {
        QString text = tr("&%1").arg( strippedName(files[i]) );
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
        recentFileActs[i]->setStatusTip(files[i]);
    }

    for( qsizetype j = numRecentFiles; j < MaxRecentFiles; ++j )
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

void ImageWindow::deleteList()
{
    QStringList files;
    files.clear();

    QSettings settings;
    settings.setValue("Main/Name/recentFileList", files);

    updateRecentFileActions();
}

void ImageWindow::updateCameraAction()
{
    auto cameras = QMediaDevices::videoInputs();

    if ( cameras.isEmpty() )
        cameraSessionAct->setEnabled(false);
    else
        cameraSessionAct->setEnabled(true);
}

void ImageWindow::onStartCameraActionTriggered()
{
    if ( !cameraSessionAct )
        return;

    if ( !camera_window )
        return;

    cameraSessionAct->setChecked(camera_window && camera_window->isVisible());

    auto cameras = QMediaDevices::videoInputs();

    if( cameras.isEmpty() )
    {
        QMessageBox::information(this,
                                 tr("Information"),
                                 tr("No camera available."));
    }
    else
    {
        camera_window->setWindowState(
            camera_window->windowState() & ~Qt::WindowMinimized
            );

        camera_window->show();
        camera_window->raise();
        camera_window->activateWindow();
    }
}

void ImageWindow::refreshAlgoOverlay()
{
    //if (imageOverlay)
        //imageOverlay->setStats(acWorker->currentStats());
}

void ImageWindow::closeEvent(QCloseEvent* event)
{
    auto& config = AppSettings::instance();
    QSettings settings;

    settings.setValue( "Main/Window/geometry", saveGeometry() );
    settings.setValue( "Main/Name/last_directory_used", last_directory_used);

    config.save();

    QMainWindow::closeEvent(event);

    QApplication::quit();
}

void ImageWindow::onDisplayedImageReady(const QImage& displayed)
{
    m_imageSize = displayed.size();
    m_channels  = displayed.depth() / 8;

    if ( m_channels > 3 )
        m_channels = 3; // because this application processes only 3 channels
                        // even there are 4 channels, for example.

    updateWindowTitle();
}

void ImageWindow::onFileSelected(const QString& path)
{
    setCurrentFile(path); // for recent file

    m_fileName = strippedName(path);
    m_fullPath = path;

    updateWindowTitle();

    statusBar()->showMessage(path);
}

void ImageWindow::updateWindowTitle()
{
    QString title = "Ofeli";

    if (    !m_fileName.isEmpty()
         && m_imageSize.isValid()
         && m_channels > 0 )
    {
        const QString colorText = (m_channels == 1) ? "Gray" : "RGB";

        const QString sizeStr = QString("%1×%2")
                                    .arg(m_imageSize.width())
                                    .arg(m_imageSize.height());

        title += QString(" - %1 - %2 - %3")
                     .arg(m_fileName, sizeStr, colorText);
    }

    setWindowTitle(title);
}

void ImageWindow::saveDisplayed()
{
    QImage displayed = imageView->renderToImage();
    if (displayed.isNull())
        return;

    QString baseName = m_fileName.isEmpty()
                           ? "displayed"
                           : QFileInfo(m_fileName).baseName();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save displayed image"),
        last_directory_used + "/" + baseName,
        tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)"),
        &selectedFilter
        );

    if (fileName.isEmpty())
        return; // Cancel

    // déduire l’extension à partir du filtre
    QString extension;
    if      (selectedFilter.contains("*.png")) extension = "png";
    else if (selectedFilter.contains("*.jpg")) extension = "jpg";
    else if (selectedFilter.contains("*.bmp")) extension = "bmp";
    else if (selectedFilter.contains("*.ppm")) extension = "ppm";
    else if (selectedFilter.contains("*.xbm")) extension = "xbm";
    else if (selectedFilter.contains("*.xpm")) extension = "xpm";

    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty())
        fileName += "." + extension;

    fileName = makeUniqueFileName( fileName );
    displayed.save(fileName);
}

QString ImageWindow::makeUniqueFileName(const QString& filePath)
{
    QFileInfo fi(filePath);
    QString base = fi.completeBaseName();
    QString ext  = fi.suffix();
    QDir dir = fi.dir();

    QString candidate = filePath;
    int index = 1;

    while (QFile::exists(candidate))
    {
        candidate = dir.filePath(
            QString("%1 (%2).%3")
                .arg(base)
                .arg(index++)
                .arg(ext)
            );
    }

    return candidate;
}

void ImageWindow::onStateChanged(ofeli_app::WorkerState state)
{
    bool isEnable = ( state != WorkerState::Uninitialized &&
                      state != WorkerState::Initializing );

    restartButton->setEnabled( isEnable );
    togglePauseButton->setEnabled( isEnable );
    stepButton->setEnabled( isEnable );
    convergeButton->setEnabled( isEnable );


    if ( state == WorkerState::Running ||
         state == WorkerState::Suspended )
    {
        restartButton->setText( tr("Restart") );
        restartButton->setToolTip(tr("Restart the active contour from its initial state."));
        restartButton->setIcon( restartIcon );
    }
    else if ( state == WorkerState::Ready )
    {
        restartButton->setText( tr("Start") );
        restartButton->setToolTip(tr("Run the active contour."));
        restartButton->setIcon( startResumeIcon );
    }

    if ( state == WorkerState::Running )
    {
        togglePauseButton->setText( tr("Pause") );
        togglePauseButton->setToolTip(tr("Suspend execution and display the current state."));
        togglePauseButton->setIcon( pauseIcon );
    }
    else if ( state == WorkerState::Suspended ||
              state == WorkerState::Ready )
    {
        togglePauseButton->setText( tr("Resume") );
        togglePauseButton->setToolTip(tr("Resume the active contour execution."));
        togglePauseButton->setIcon( startResumeIcon );
    }
}

void ImageWindow::onCameraWindowShown()
{
    cameraSessionAct->setChecked(true);
}

void ImageWindow::onCameraWindowClosed()
{
    cameraSessionAct->setChecked(false);
}

}
