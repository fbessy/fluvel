#include "image_window.hpp"

#include "active_contour_worker.hpp"
#include "algo_info_overlay.hpp"
#include "autofit_behavior.hpp"
#include "fullscreen_behavior.hpp"
#include "icon_loader.hpp"
#include "image_controller.hpp"
#include "image_view.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>

namespace ofeli_app
{

ImageWindow::ImageWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupActions();
    setupConnections();

    updateRecentFileActions();
    updateCameraAction();

    QSettings settings;
    last_directory_used_ =
        settings.value("Main/Name/last_directory_used", QDir().homePath()).toString();
}

void ImageWindow::setupUi()
{
    updateWindowTitle();
    setWindowIcon(QIcon(":/icons/app/Ofeli.svg"));

    QSettings settings;

    const auto geo = settings.value("Main/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    startResumeIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackStart, QStyle::SP_MediaPlay,
                                    ":/icons/toolbar/media-playback-start-symbolic.svg");

    restartIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaylistRepeat, QStyle::SP_BrowserReload,
                                ":/icons/toolbar/media-playlist-repeat-symbolic.svg");

    pauseIcon_ = il::loadIcon(QIcon::ThemeIcon::MediaPlaybackPause, QStyle::SP_MediaPause,
                              ":/icons/toolbar/media-playback-pause-symbolic.svg");

    restartButton_ = new QPushButton(tr("Start"));
    restartButton_->setToolTip(tr("Run the active contour."));
    restartButton_->setIcon(startResumeIcon_);

    togglePauseButton_ = new QPushButton(tr("Resume"));
    togglePauseButton_->setToolTip(tr("Resume the active contour execution."));
    togglePauseButton_->setIcon(startResumeIcon_);

    stepButton_ = new QPushButton(tr("Step"));
    stepButton_->setToolTip(tr("Advance the active contour by one iteration."));

    QIcon stepIcon = il::loadIcon(QIcon::ThemeIcon::GoNext, QStyle::SP_ArrowRight,
                                  ":/icons/toolbar/go-next-symbolic.svg");

    stepButton_->setIcon(stepIcon);

    stepButton_->setAutoRepeat(true);
    stepButton_->setAutoRepeatDelay(300);
    stepButton_->setAutoRepeatInterval(100);

    convergeButton_ = new QPushButton(tr("Converge"));
    convergeButton_->setToolTip(tr("Run until completion without displaying intermediate steps."));

    QIcon convergeIcon =
        il::loadIcon(QIcon::ThemeIcon::MediaSeekForward, QStyle::SP_MediaSeekForward,
                     ":/icons/toolbar/media-seek-forward-symbolic.svg");

    convergeButton_->setIcon(convergeIcon);

    restartButton_->setEnabled(false);
    togglePauseButton_->setEnabled(false);
    stepButton_->setEnabled(false);
    convergeButton_->setEnabled(false);

    rightPanelToggle_ = new RightPanelToggleButton;

    settingsButton_ = new QPushButton;
    settingsButton_->setToolTip(tr("Image session settings"));
    settingsButton_->setFlat(true);
    settingsButton_->setFocusPolicy(Qt::NoFocus);

    settingsIcon_ = il::loadIcon("configure", ":/icons/toolbar/configure-symbolic.svg");

    settingsButton_->setIcon(settingsIcon_);

    // settingsButton->setAutoDefault(false);
    // settingsButton->setFlat(true);

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

    controlLayout->addWidget(restartButton_);
    controlLayout->addWidget(togglePauseButton_);
    controlLayout->addWidget(stepButton_);
    controlLayout->addWidget(convergeButton_);
    controlLayout->addStretch();
    controlLayout->addWidget(rightPanelToggle_);
    controlLayout->addSpacerItem(new QSpacerItem(12, 0, QSizePolicy::Fixed, QSizePolicy::Minimum));
    controlLayout->addWidget(settingsButton_);

    // --- Image view ---
    imageView_ = new ImageView(AppSettings::instance().imgConfig.display,
                               AppSettings::instance().imgConfig.compute.downscale, central);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    imageView_->setInteraction(interaction.release());

    imageOverlay_ = new AlgoInfoOverlay(imageView_->viewport());
    imageOverlay_->raise();

    // --- Display bar (à droite) ---
    displayBar_ = new DisplaySettingsWidget(central, Session::Image);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(imageView_, 1);  // prend tout l'espace
    contentLayout->addWidget(displayBar_, 0); // largeur naturelle

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

    imageSessionAct_ = new QAction(tr("&Image"), this);
    imageSessionAct_->setShortcut(tr("Ctrl+I"));

    QIcon imageIcon =
        il::loadIcon("image-x-generic-symbolic", ":/icons/toolbar/view-preview-symbolic.svg");

    imageSessionAct_->setIcon(imageIcon);
    imageSessionAct_->setCheckable(true);
    imageSessionAct_->setChecked(true);
    imageSessionAct_->setEnabled(true);

    cameraSessionAct_ = new QAction(tr("Came&ra"), this);
    cameraSessionAct_->setShortcut(tr("Ctrl+R"));

    QIcon cameraIcon =
        il::loadIcon(QIcon::ThemeIcon::CameraWeb, ":/icons/toolbar/camera-web-symbolic.svg");

    cameraSessionAct_->setIcon(cameraIcon);

    cameraSessionAct_->setCheckable(true);
    cameraSessionAct_->setChecked(false);
    cameraSessionAct_->setEnabled(false);

    quitAct_ = new QAction(tr("&Quit"), this);
    quitAct_->setShortcut(QKeySequence::Quit);

    QIcon quitIcon = il::loadIcon(QIcon::ThemeIcon::ApplicationExit, QStyle::SP_TitleBarCloseButton,
                                  ":/icons/toolbar/application-exit-symbolic.svg");

    quitAct_->setIcon(quitIcon);

    openAct_ = new QAction(tr("&Open..."), this);
    openAct_->setShortcut(QKeySequence::Open);
    openAct_->setStatusTip(
        tr("Open an image file (*.png, *.bmp, *.jpg, *.jpeg, *.tiff, *.tif, *.gif, *.pbm, "
           "*.pgm, *.ppm, *.svg, *.svgz, *.mng, *.xbm, *.xpm)."));

    QIcon openIcon = il::loadIcon(QIcon::ThemeIcon::DocumentOpen, QStyle::SP_DirOpenIcon,
                                  ":/icons/toolbar/document-open-symbolic.svg");

    openAct_->setIcon(openIcon);

    deleteAct_ = new QAction(tr("Clear list"), this);
    deleteAct_->setStatusTip(tr("Clean the recent files list."));

    QIcon deleteIcon = il::loadIcon(QIcon::ThemeIcon::EditClear, QStyle::SP_LineEditClearButton,
                                    ":/icons/toolbar/edit-clear-history.svg");

    deleteAct_->setIcon(deleteIcon);

    saveAct_ = new QAction(tr("&Save..."), this);
    saveAct_->setShortcut(QKeySequence::Save);
    saveAct_->setStatusTip(tr("Save the displayed image."));

    QIcon saveIcon = il::loadIcon(QIcon::ThemeIcon::DocumentSaveAs, QStyle::SP_DialogSaveButton,
                                  ":/icons/toolbar/document-save-as-symbolic.svg");

    saveAct_->setIcon(saveIcon);

    mediaDevices_ = new QMediaDevices(this);

    QIcon recentIcon = il::loadIcon(QIcon::ThemeIcon::DocumentOpenRecent, QStyle::SP_DirOpenIcon,
                                    ":/icons/toolbar/document-open-recent-symbolic.svg");

    for (int i = 0; i < kMaxRecentFiles; ++i)
    {
        recentFileActs_[i] = new QAction(this);
        recentFileActs_[i]->setVisible(false);

        recentFileActs_[i]->setIcon(recentIcon);
    }

    analysisAct_ = new QAction(tr("&Analysis"), this);
    analysisAct_->setStatusTip(tr("Compute the Hausdorff distance."));
    analysisAct_->setShortcut(tr("Ctrl+A"));

    analysisAct_->setIcon(QIcon(":/icons/toolbar/measure-symbolic.svg"));

    settingsAct_ = new QAction(tr("&Settings"), this);
    settingsAct_->setShortcut(QKeySequence::Preferences);
    settingsAct_->setStatusTip(tr("Image preprocessing and active contour initialization."));
    settingsAct_->setEnabled(true);

    settingsAct_->setIcon(settingsIcon_);

    menuBar()->addSeparator();

    aboutAct_ = new QAction(tr("&About"), this);
    aboutAct_->setStatusTip(tr("Information, license and home page."));

    QIcon aboutIcon = il::loadIcon(QIcon::ThemeIcon::HelpAbout, QStyle::SP_MessageBoxInformation,
                                   ":/icons/toolbar/help-about-symbolic.svg");

    aboutAct_->setIcon(aboutIcon);

    languageAct_ = new QAction(tr("&Language"), this);
    languageAct_->setStatusTip(tr("Choose the application language."));

    QIcon languageIcon = il::loadIcon("preferences-desktop-locale",
                                      ":/icons/toolbar/preferences-desktop-locale.svg");

    languageAct_->setIcon(languageIcon);

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////                          Create Menus                /////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    sessionMenu_ = new QMenu(tr("&Session"), this);
    sessionMenu_->addAction(imageSessionAct_);
    sessionMenu_->addAction(cameraSessionAct_);
    sessionMenu_->addSeparator();
    sessionMenu_->addAction(quitAct_);

    fileMenu_ = new QMenu(tr("&File"), this);
    fileMenu_->addAction(openAct_);

    separatorAct_ = fileMenu_->addSeparator();
    for (int i = 0; i < kMaxRecentFiles; ++i)
    {
        fileMenu_->addAction(recentFileActs_[i]);
    }

    fileMenu_->addAction(deleteAct_);

    fileMenu_->addSeparator();

    fileMenu_->addAction(saveAct_);

    segmentationMenu_ = new QMenu(tr("&Segmentation"), this);
    segmentationMenu_->addAction(analysisAct_);
    segmentationMenu_->addAction(settingsAct_);

    helpMenu_ = new QMenu(tr("&Help"), this);
    helpMenu_->addAction(aboutAct_);
    helpMenu_->addAction(languageAct_);

    menuBar()->addMenu(sessionMenu_);
    menuBar()->addMenu(fileMenu_);
    menuBar()->addMenu(segmentationMenu_);
    menuBar()->addMenu(helpMenu_);

    imageController_ = new ImageController(this);

    settings_window_ = new SettingsWindow(this);
    camera_window_ = new CameraWindow;
    evaluation_window_ = new AnalysisWindow(this);
    about_window_ = new AboutWindow(this);
    language_window_ = new LanguageWindow(this);
}

void ImageWindow::setupConnections()
{
    connect(this, &ImageWindow::fileSelected, this, &ImageWindow::onFileSelected);

    connect(imageController_, &ImageController::displayedImageReady, this,
            &ImageWindow::onDisplayedImageReady);

    nameFilters_ << "*.bmp"
                 << "*.gif"
                 << "*.jpg" << "*.jpeg" << "*.mng"
                 << "*.pbm" << "*.png" << "*.pgm"
                 << "*.ppm" << "*.svg" << "*.svgz"
                 << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    nameFilters_.removeDuplicates();

    connect(openAct_, &QAction::triggered, this,
            [this]()
            {
                QString fileName = QFileDialog::getOpenFileName(
                    this, tr("Open Image"), last_directory_used_,
                    tr("Image Files (%1)").arg(nameFilters_.join(" ")));
                if (!fileName.isEmpty())
                {
                    last_directory_used_ = QFileInfo(fileName).absolutePath();
                    emit fileSelected(fileName);
                }
            });

    for (int i = 0; i < kMaxRecentFiles; ++i)
    {
        connect(recentFileActs_[i], &QAction::triggered, this,
                [this]()
                {
                    QAction* action = qobject_cast<QAction*>(sender());
                    QString fileName;
                    if (action != nullptr)
                    {
                        fileName = action->data().toString();
                    }
                    if (!fileName.isEmpty())
                    {
                        emit fileSelected(fileName);
                    }
                });
    }
    connect(this, &ImageWindow::fileSelected, imageController_, &ImageController::loadImage);

    connect(deleteAct_, &QAction::triggered, this, &ImageWindow::deleteList);

    connect(saveAct_, &QAction::triggered, this, &ImageWindow::saveDisplayed);

    connect(quitAct_, &QAction::triggered, this, &ImageWindow::close);

    connect(imageSessionAct_, &QAction::triggered, this,
            [this]()
            {
                imageSessionAct_->setChecked(true);
            });

    connect(cameraSessionAct_, &QAction::triggered, this,
            &ImageWindow::onStartCameraActionTriggered);

    connect(camera_window_, &CameraWindow::cameraWindowClosed, this,
            &ImageWindow::onCameraWindowClosed);
    connect(camera_window_, &CameraWindow::cameraWindowShown, this,
            &ImageWindow::onCameraWindowShown);

    connect(mediaDevices_, &QMediaDevices::videoInputsChanged, this,
            &ImageWindow::updateCameraAction);

    connect(analysisAct_, &QAction::triggered, evaluation_window_, &AnalysisWindow::show);

    connect(settingsAct_, &QAction::triggered, settings_window_, &SettingsWindow::show);

    connect(aboutAct_, &QAction::triggered, about_window_, &AboutWindow::show);

    connect(languageAct_, &QAction::triggered, language_window_, &LanguageWindow::show);

    imageView_->applyDownscaleConfig(AppSettings::instance().imgConfig.compute.downscale);
    imageView_->applyDisplayConfig(AppSettings::instance().imgConfig.display);

    connect(&AppSettings::instance(), &ApplicationSettings::imgSettingsChanged, this,
            [this](const ImageSessionSettings& conf)
            {
                imageView_->applyDownscaleConfig(conf.compute.downscale);
            });

    connect(&AppSettings::instance(), &ApplicationSettings::imgDisplaySettingsChanged, imageView_,
            &ImageView::applyDisplayConfig);

    connect(imageController_, &ImageController::clearOverlaysRequested, imageView_,
            &ImageView::clearOverlays);

    connect(imageController_, &ImageController::displayedImageReady, imageView_,
            &ImageView::setImage);

    connect(imageController_, &ImageController::inputImageReady, this,
            &ImageWindow::onInputImageReady);

    connect(this, &ImageWindow::inputImageReady, settings_window_,
            &SettingsWindow::onInputImageReady);

    connect(imageController_, &ImageController::contourUpdated, imageView_, &ImageView::setContour,
            Qt::QueuedConnection);

    QTimer* m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(30); // ~33 FPS
    connect(m_statsTimer, &QTimer::timeout, this, &ImageWindow::refreshAlgoOverlay);
    m_statsTimer->start();

    connect(restartButton_, &QPushButton::clicked, imageController_, &ImageController::restart);

    connect(togglePauseButton_, &QPushButton::clicked, imageController_,
            &ImageController::togglePause);

    connect(stepButton_, &QPushButton::clicked, imageController_, &ImageController::step);

    connect(convergeButton_, &QPushButton::clicked, imageController_, &ImageController::converge);

    connect(rightPanelToggle_, &QPushButton::toggled, displayBar_,
            &DisplaySettingsWidget::setPanelVisible);

    connect(settingsButton_, &QPushButton::clicked, settings_window_, &SettingsWindow::show);

    connect(imageController_, &ImageController::stateChanged, this, &ImageWindow::onStateChanged);
}

QString ImageWindow::strippedName(const QString& fullFilename)
{
    return QFileInfo(fullFilename).fileName();
}

void ImageWindow::setCurrentFile(const QString& fileName)
{
    QSettings settings;
    QStringList files = settings.value("Main/Name/recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > kMaxRecentFiles)
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

    qsizetype numRecentFiles = qMin(files.size(), kMaxRecentFiles);

    for (qsizetype i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1").arg(strippedName(files[i]));
        recentFileActs_[i]->setText(text);
        recentFileActs_[i]->setData(files[i]);
        recentFileActs_[i]->setVisible(true);
        recentFileActs_[i]->setStatusTip(files[i]);
    }

    for (qsizetype j = numRecentFiles; j < kMaxRecentFiles; ++j)
    {
        recentFileActs_[j]->setVisible(false);
    }

    separatorAct_->setVisible(numRecentFiles > 0);

    if (files.isEmpty())
    {
        deleteAct_->setVisible(false);
    }
    else
    {
        deleteAct_->setVisible(true);
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

    if (cameras.isEmpty())
        cameraSessionAct_->setEnabled(false);
    else
        cameraSessionAct_->setEnabled(true);
}

void ImageWindow::onStartCameraActionTriggered()
{
    if (!cameraSessionAct_)
        return;

    if (!camera_window_)
        return;

    cameraSessionAct_->setChecked(camera_window_ && camera_window_->isVisible());

    auto cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty())
    {
        QMessageBox::information(this, tr("Information"), tr("No camera available."));
    }
    else
    {
        camera_window_->setWindowState(camera_window_->windowState() & ~Qt::WindowMinimized);

        camera_window_->show();
        camera_window_->raise();
        camera_window_->activateWindow();
    }
}

void ImageWindow::refreshAlgoOverlay()
{
    // if (imageOverlay)
    // imageOverlay->setStats(acWorker->currentStats());
}

void ImageWindow::closeEvent(QCloseEvent* event)
{
    auto& config = AppSettings::instance();
    QSettings settings;

    settings.setValue("Main/Window/geometry", saveGeometry());
    settings.setValue("Main/Name/last_directory_used", last_directory_used_);

    config.save();

    QMainWindow::closeEvent(event);

    QApplication::quit();
}

void ImageWindow::onDisplayedImageReady(const QImage& displayed)
{
    m_imageSize_ = displayed.size();
    m_channels_ = displayed.depth() / 8;

    if (m_channels_ > 3)
        m_channels_ = 3; // because this application processes only 3 channels
                         // even there are 4 channels, for example.

    updateWindowTitle();
}

void ImageWindow::onFileSelected(const QString& path)
{
    setCurrentFile(path); // for recent file

    m_fileName_ = strippedName(path);
    m_fullPath_ = path;

    updateWindowTitle();

    statusBar()->showMessage(path);
}

void ImageWindow::updateWindowTitle()
{
    QString title = "Ofeli";

    if (!m_fileName_.isEmpty() && m_imageSize_.isValid() && m_channels_ > 0)
    {
        const QString colorText = (m_channels_ == 1) ? "Gray" : "RGB";

        const QString sizeStr =
            QString("%1×%2").arg(m_imageSize_.width()).arg(m_imageSize_.height());

        title += QString(" - %1 - %2 - %3").arg(m_fileName_, sizeStr, colorText);
    }

    setWindowTitle(title);
}

void ImageWindow::saveDisplayed()
{
    QImage displayed = imageView_->renderToImage();
    if (displayed.isNull())
        return;

    QString baseName = m_fileName_.isEmpty() ? "displayed" : QFileInfo(m_fileName_).baseName();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save displayed image"), last_directory_used_ + "/" + baseName,
        tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)"),
        &selectedFilter);

    if (fileName.isEmpty())
        return; // Cancel

    // déduire l’extension à partir du filtre
    QString extension;
    if (selectedFilter.contains("*.png"))
        extension = "png";
    else if (selectedFilter.contains("*.jpg"))
        extension = "jpg";
    else if (selectedFilter.contains("*.bmp"))
        extension = "bmp";
    else if (selectedFilter.contains("*.ppm"))
        extension = "ppm";
    else if (selectedFilter.contains("*.xbm"))
        extension = "xbm";
    else if (selectedFilter.contains("*.xpm"))
        extension = "xpm";

    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty())
        fileName += "." + extension;

    fileName = makeUniqueFileName(fileName);
    displayed.save(fileName);
}

QString ImageWindow::makeUniqueFileName(const QString& filePath)
{
    QFileInfo fi(filePath);
    QString base = fi.completeBaseName();
    QString ext = fi.suffix();
    QDir dir = fi.dir();

    QString candidate = filePath;
    int index = 1;

    while (QFile::exists(candidate))
    {
        candidate = dir.filePath(QString("%1 (%2).%3").arg(base).arg(index++).arg(ext));
    }

    return candidate;
}

void ImageWindow::onStateChanged(ofeli_app::WorkerState state)
{
    bool isEnable = (state != WorkerState::Uninitialized && state != WorkerState::Initializing);

    restartButton_->setEnabled(isEnable);
    togglePauseButton_->setEnabled(isEnable);
    stepButton_->setEnabled(isEnable);
    convergeButton_->setEnabled(isEnable);

    if (state == WorkerState::Running || state == WorkerState::Suspended)
    {
        restartButton_->setText(tr("Restart"));
        restartButton_->setToolTip(tr("Restart the active contour from its initial state."));
        restartButton_->setIcon(restartIcon_);
    }
    else if (state == WorkerState::Ready)
    {
        restartButton_->setText(tr("Start"));
        restartButton_->setToolTip(tr("Run the active contour."));
        restartButton_->setIcon(startResumeIcon_);
    }

    if (state == WorkerState::Running)
    {
        togglePauseButton_->setText(tr("Pause"));
        togglePauseButton_->setToolTip(tr("Suspend execution and display the current state."));
        togglePauseButton_->setIcon(pauseIcon_);
    }
    else if (state == WorkerState::Suspended || state == WorkerState::Ready)
    {
        togglePauseButton_->setText(tr("Resume"));
        togglePauseButton_->setToolTip(tr("Resume the active contour execution."));
        togglePauseButton_->setIcon(startResumeIcon_);
    }
}

void ImageWindow::onCameraWindowShown()
{
    cameraSessionAct_->setChecked(true);
}

void ImageWindow::onCameraWindowClosed()
{
    cameraSessionAct_->setChecked(false);
}

void ImageWindow::onInputImageReady(const QImage& inputImage)
{
    emit inputImageReady(inputImage);
}

} // namespace ofeli_app
