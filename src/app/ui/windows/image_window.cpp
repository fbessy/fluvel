// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_window.hpp"

#include "about_window.hpp"
#include "analysis_window.hpp"
#include "application_settings.hpp"
#include "camera_window.hpp"
#include "language_window.hpp"
#include "settings_window.hpp"

#include "display_settings_widget.hpp"
#include "icon_loader.hpp"
#include "right_panel_toggle_button.hpp"

#include "autofit_behavior.hpp"
#include "drag_drop_behavior.hpp"
#include "fullscreen_behavior.hpp"
#include "image_viewer_widget.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "pixel_info_behavior.hpp"

#include "file_utils.hpp"
#include "image_controller.hpp"

#include <QApplication>
#include <QCameraDevice>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImageReader>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace fluvel_app
{

ImageWindow::ImageWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupActions();
    setupMenus();
    setupControllers();
    setupChildWindows();

    applyInitialSettings();

    setupConnections();

    QSettings settings;
    lastDirectoryUsed_ = settings.value("history/last_directory", QDir().homePath()).toString();
}

void ImageWindow::setupUi()
{
    updateWindowTitle();

    QSettings settings;

    if (settings.contains("ui_geometry/image_window"))
    {
        restoreGeometry(settings.value("ui_geometry/image_window").toByteArray());
    }
    else
    {
        resize(900, 600);
    }

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

    const auto& config = ApplicationSettings::instance().imageSettings();
    imageViewer_ = new ImageViewerWidget(config.display, config.compute.downscale, central);
    imageViewer_->setMaxDisplayFps(60.0);

    auto interaction = std::make_unique<InteractionSet>();
    interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<PixelInfoBehavior>());
    interaction->addBehavior(std::make_unique<DragDropBehavior>());
    imageViewer_->setInteraction(interaction.release());

    // --- Display bar (à droite) ---
    displayBar_ = new DisplaySettingsWidget(config.display, central);

    // --- Layout horizontal contenu principal ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(imageViewer_, 1); // prend tout l'espace
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

    clearAct_ = new QAction(tr("Clear list"), this);
    clearAct_->setStatusTip(tr("Clean the recent files list."));

    QIcon deleteIcon = il::loadIcon(QIcon::ThemeIcon::EditClear, QStyle::SP_LineEditClearButton,
                                    ":/icons/toolbar/edit-clear-history.svg");

    clearAct_->setIcon(deleteIcon);

    saveAct_ = new QAction(tr("&Save..."), this);
    saveAct_->setShortcut(QKeySequence::Save);
    saveAct_->setStatusTip(tr("Save the displayed image."));

    QIcon saveIcon = il::loadIcon(QIcon::ThemeIcon::DocumentSaveAs, QStyle::SP_DialogSaveButton,
                                  ":/icons/toolbar/document-save-as-symbolic.svg");

    saveAct_->setIcon(saveIcon);

    QIcon recentIcon = il::loadIcon(QIcon::ThemeIcon::DocumentOpenRecent, QStyle::SP_DirOpenIcon,
                                    ":/icons/toolbar/document-open-recent-symbolic.svg");

    for (auto& act : recentFileActs_)
    {
        act = new QAction(this);
        act->setVisible(false);

        act->setIcon(recentIcon);
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
}

void ImageWindow::setupMenus()
{
    sessionMenu_ = new QMenu(tr("&Session"), this);
    sessionMenu_->addAction(imageSessionAct_);
    sessionMenu_->addAction(cameraSessionAct_);
    sessionMenu_->addSeparator();
    sessionMenu_->addAction(quitAct_);

    fileMenu_ = new QMenu(tr("&File"), this);
    fileMenu_->addAction(openAct_);

    separatorAct_ = fileMenu_->addSeparator();
    for (auto& act : recentFileActs_)
    {
        fileMenu_->addAction(act);
    }

    fileMenu_->addAction(clearAct_);

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
}

void ImageWindow::setupControllers()
{
    const auto& config = ApplicationSettings::instance().imageSettings();
    imageController_ = new ImageController(config, this);
}

void ImageWindow::setupChildWindows()
{
    const auto& config = ApplicationSettings::instance().imageSettings();

    settingsWindow_ = new SettingsWindow(this, config);
    cameraWindow_ = new CameraWindow(this);
    analysisWindow_ = new AnalysisWindow(this);
    aboutWindow_ = new AboutWindow(this);
    languageWindow_ = new LanguageWindow(this);
}

void ImageWindow::applyInitialSettings()
{
    assert(imageViewer_ && displayBar_);

    const auto& config = ApplicationSettings::instance().imageSettings();

    imageViewer_->applyDownscaleConfig(config.compute.downscale);
    imageViewer_->applyDisplayConfig(config.display);

    bool preprocessing =
        config.compute.downscale.hasDownscale || config.compute.processing.hasProcessing();

    displayBar_->updatePipelineAvailability(preprocessing);

    updateRecentFileActions();
    updateCameraAction(cameraWindow_->isCameraAvailable());
}

void ImageWindow::setupConnections()
{
    setupUserActionsConnections();

    // to refresh camera session state in the menu
    connect(cameraWindow_, &CameraWindow::cameraWindowShown, this,
            &ImageWindow::onCameraWindowShown);
    connect(cameraWindow_, &CameraWindow::cameraWindowClosed, this,
            &ImageWindow::onCameraWindowClosed);

    setupFileEventConnections();

    // --- Hardware events (camera devices via camera window via camera controller...) ---
    connect(cameraWindow_, &CameraWindow::cameraAvailabilityChanged, this,
            &ImageWindow::updateCameraAction);

    // --- Controller → View / Window updates ---

    // to refresh the view with a new image
    connect(imageController_, &ImageController::displayedImageReady, imageViewer_,
            &ImageViewerWidget::setImage);

    // to refresh the view with a new contour
    connect(imageController_, &ImageController::contourUpdated, imageViewer_,
            &ImageViewerWidget::setContour, Qt::QueuedConnection);

    // to refresh the view with a new text info algo overlay (mean out, iterations, ect)
    connect(imageController_, &ImageController::textDiagnosticsUpdated, imageViewer_,
            &ImageViewerWidget::setText);

    // to refresh the view and clear the former contour
    // (it's performed the first time or when a new image is loaded)
    connect(imageController_, &ImageController::clearContourRequested, imageViewer_,
            &ImageViewerWidget::clearContour);

    // to refresh the image window title
    connect(imageController_, &ImageController::displayedImageReady, this,
            &ImageWindow::onDisplayedImageReady);

    // ---  View -> Controller for display stats ---

    // --- Application settings synchronization ---

    bindApplicationSettingsToController();
    bindApplicationSettingsToView();
    bindUiToApplicationSettings();

    // to forward a new image for the image settings window view (preview)
    connect(imageController_, &ImageController::inputImageReady, settingsWindow_,
            &SettingsWindow::handleInputImageReady);

    // to retrieve worker events and refresh the buttons states (start/restart, pause/resume)
    connect(imageController_, &ImageController::stateChanged, this, &ImageWindow::onStateChanged);

    // to show an error message when a file format error occured.
    connect(imageController_, &ImageController::errorOccurred, this,
            &ImageWindow::showErrorMessage);

    // refresh widget in function of settings
    connect(&ApplicationSettings::instance(), &ApplicationSettings::imgSettingsChanged, this,
            [this](const ImageSessionSettings& conf)
            {
                bool hasPreprocessing =
                    conf.compute.downscale.hasDownscale || conf.compute.processing.hasProcessing();

                displayBar_->updatePipelineAvailability(hasPreprocessing);
            });
}

void ImageWindow::bindApplicationSettingsToController()
{
    assert(imageController_);

    const auto& config = ApplicationSettings::instance();

    connect(&config, &ApplicationSettings::imgSettingsChanged, imageController_,
            &ImageController::onImageSettingsChanged);

    connect(&config, &ApplicationSettings::imgDisplaySettingsChanged, imageController_,
            &ImageController::onImageDisplaySettingsChanged);
}

void ImageWindow::bindApplicationSettingsToView()
{
    const auto& config = ApplicationSettings::instance();

    connect(&config, &ApplicationSettings::imgSettingsChanged, this,
            [this](const ImageSessionSettings& conf)
            {
                imageViewer_->applyDownscaleConfig(conf.compute.downscale);
            });

    connect(&config, &ApplicationSettings::imgDisplaySettingsChanged, imageViewer_,
            &ImageViewerWidget::applyDisplayConfig);
}

void ImageWindow::bindUiToApplicationSettings()
{
    const auto& config = ApplicationSettings::instance();

    connect(settingsWindow_, &SettingsWindow::imageSessionSettingsAccepted, &config,
            &ApplicationSettings::setImageSessionSettings);

    connect(displayBar_, &DisplaySettingsWidget::displayConfigChanged, &config,
            &ApplicationSettings::setImageDisplayConfig);
}

void ImageWindow::setupUserActionsConnections()
{
    // ---   1st menu   ---

    connect(imageSessionAct_, &QAction::triggered, this,
            [this]()
            {
                imageSessionAct_->setChecked(true);
            });

    connect(cameraSessionAct_, &QAction::triggered, this,
            &ImageWindow::onStartCameraActionTriggered);

    connect(quitAct_, &QAction::triggered, this, &ImageWindow::close);

    // ---   2nd menu   ---

    connect(openAct_, &QAction::triggered, this,
            [this]()
            {
                QString fileName = QFileDialog::getOpenFileName(
                    this, tr("Open Image"), lastDirectoryUsed_, file_utils::buildImageFilter());

                if (!fileName.isEmpty())
                    emit fileSelected(fileName);
            });

    for (auto* act : recentFileActs_)
    {
        connect(act, &QAction::triggered, this,
                [this, act]()
                {
                    const QString fileName = act->data().toString();
                    if (!fileName.isEmpty())
                        emit fileSelected(fileName);
                });
    }

    connect(clearAct_, &QAction::triggered, this, &ImageWindow::clearRecentFiles);

    connect(saveAct_, &QAction::triggered, this, &ImageWindow::saveDisplayed);

    // ---   3rd menu   ---

    connect(analysisAct_, &QAction::triggered, analysisWindow_, &AnalysisWindow::show);

    connect(settingsAct_, &QAction::triggered, settingsWindow_, &SettingsWindow::show);

    // ---   4th menu   ---

    connect(aboutAct_, &QAction::triggered, aboutWindow_, &AboutWindow::show);

    connect(languageAct_, &QAction::triggered, languageWindow_, &LanguageWindow::show);

    // ---   6 buttons   ---

    connect(restartButton_, &QPushButton::clicked, imageController_, &ImageController::restart);

    connect(togglePauseButton_, &QPushButton::clicked, imageController_,
            &ImageController::togglePause);

    connect(stepButton_, &QPushButton::clicked, imageController_, &ImageController::step);

    connect(convergeButton_, &QPushButton::clicked, imageController_, &ImageController::converge);

    connect(rightPanelToggle_, &QPushButton::toggled, displayBar_,
            &DisplaySettingsWidget::setPanelVisible);

    connect(settingsButton_, &QPushButton::clicked, settingsWindow_, &SettingsWindow::show);

    // when the user drag and drop an image in the view of the image window.
    connect(imageViewer_, &ImageViewerWidget::imageDropped, imageController_,
            &ImageController::loadImage);
}

void ImageWindow::setupFileEventConnections()
{
    connect(this, &ImageWindow::fileSelected, imageController_, &ImageController::loadImage);

    connect(imageController_, &ImageController::imageOpened, this, &ImageWindow::onFileOpened);
}

void ImageWindow::setCurrentFile(const QString& fileName)
{
    QSettings settings;
    QStringList files = settings.value("history/recent_files").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > kMaxRecentFiles)
    {
        files.removeLast();
    }

    settings.setValue("history/recent_files", files);
    updateRecentFileActions();
}

void ImageWindow::updateRecentFileActions()
{
    QSettings settings;
    const auto files = settings.value("history/recent_files").toStringList();

    const std::size_t numRecentFiles =
        std::min<std::size_t>(static_cast<std::size_t>(files.size()), kMaxRecentFiles);

    // 🔥 D'abord tout cacher
    for (auto& act : recentFileActs_)
    {
        act->setVisible(false);
    }

    // Ensuite afficher ce qu'il faut
    for (std::size_t i = 0; i < numRecentFiles; ++i)
    {
        const QString& file = files[static_cast<qsizetype>(i)];

        QString text = tr("&%1").arg(file_utils::strippedName(file));

        recentFileActs_[i]->setText(text);
        recentFileActs_[i]->setData(file);
        recentFileActs_[i]->setVisible(true);
        recentFileActs_[i]->setStatusTip(file);
    }

    separatorAct_->setVisible(numRecentFiles > 0);
    clearAct_->setVisible(!files.isEmpty());
}

void ImageWindow::clearRecentFiles()
{
    QSettings settings;
    settings.setValue("history/recent_files", QStringList());

    updateRecentFileActions();
}

void ImageWindow::updateCameraAction(bool available)
{
    assert(cameraSessionAct_);

    cameraSessionAct_->setEnabled(available);
}

void ImageWindow::onStartCameraActionTriggered()
{
    if (!cameraSessionAct_)
        return;

    if (!cameraWindow_)
        return;

    if (!cameraWindow_->isCameraAvailable())
    {
        QMessageBox::information(this, tr("Information"), tr("No camera available."));
    }
    else
    {
        cameraSessionAct_->setChecked(true);

        cameraWindow_->setWindowState(cameraWindow_->windowState() & ~Qt::WindowMinimized);

        cameraWindow_->show();
        cameraWindow_->raise();
        cameraWindow_->activateWindow();
    }
}

void ImageWindow::closeEvent(QCloseEvent* event)
{
    auto& config = ApplicationSettings::instance();
    QSettings settings;

    settings.setValue("ui_geometry/image_window", saveGeometry());
    settings.setValue("history/last_directory", lastDirectoryUsed_);

    config.saveQuiet();

    QMainWindow::closeEvent(event);

    QApplication::quit();
}

void ImageWindow::onDisplayedImageReady(const QImage& displayed)
{
    imageSize_ = displayed.size();
    channels_ = displayed.depth() / 8;

    if (channels_ > 3)
        channels_ = 3; // because this application processes only 3 channels
                       // even there are 4 channels, for example.

    updateWindowTitle();
}

void ImageWindow::onFileOpened(const QString& path)
{
    if (path.isEmpty())
        return;

    setCurrentFile(path); // for recent file

    lastDirectoryUsed_ = QFileInfo(path).absolutePath();

    fileName_ = file_utils::strippedName(path);
    fullPath_ = path;

    updateWindowTitle();

    statusBar()->showMessage(path);
}

void ImageWindow::updateWindowTitle()
{
    QString title = "Fluvel";

    if (!fileName_.isEmpty() && imageSize_.isValid() && channels_ > 0)
    {
        const QString colorText = (channels_ == 1) ? "Gray" : "RGB";

        const QString sizeStr = QString("%1×%2").arg(imageSize_.width()).arg(imageSize_.height());

        title += QString(" — %1 - %2 - %3").arg(fileName_, sizeStr, colorText);
    }

    setWindowTitle(title);
}

void ImageWindow::saveDisplayed()
{
    QImage displayed = imageViewer_->renderToImage();
    if (displayed.isNull())
        return;

    QString baseName = fileName_.isEmpty() ? "displayed" : QFileInfo(fileName_).baseName();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save displayed image"), lastDirectoryUsed_ + "/" + baseName,
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

    fileName = file_utils::makeUniqueFileName(fileName);
    displayed.save(fileName);
}

void ImageWindow::onStateChanged(fluvel_app::WorkerState state)
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

void ImageWindow::showErrorMessage(const QString& msg)
{
    QMessageBox::warning(this, tr("Error"), msg);
}

void ImageWindow::showEvent(QShowEvent* event)
{
    // Called here (instead of setupUI) to ensure the window is fully
    // initialized before setting the icon. This avoids a pixelated icon rendering
    // issue observed with Qt/KDE on Wayland (Plasma) until the window decorations
    // are refreshed.
    QTimer::singleShot(0, this,
                       [this]()
                       {
                           setWindowIcon(QIcon(":/icons/app/fluvel.svg"));
                       });

    QMainWindow::showEvent(event);
}

} // namespace fluvel_app
