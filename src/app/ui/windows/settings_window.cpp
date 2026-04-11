// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "settings_window.hpp"

#include "algo_settings_widget.hpp"
#include "image_pipeline.hpp"
#include "image_viewer_widget.hpp"
#include "initialization_behavior.hpp"
#include "interaction_set.hpp"
#include "kernel_size_spinbox.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSlider>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

#ifdef FLUVEL_DEBUG
#include "image_debug.hpp"
#endif

namespace fluvel_app
{

SettingsWindow::SettingsWindow(QWidget* parent, const ImageSessionSettings& config)
    : QDialog(parent)
    , committedConfig_(config)
    , editedDownscaleConfig_(config.compute.downscale)
    , editedProcessingConfig_(config.compute.processing)
{
    setWindowTitle(tr("Image session settings"));

    QSettings settings;

    if (settings.contains("ui_geometry/settings_window"))
    {
        restoreGeometry(settings.value("ui_geometry/settings_window").toByteArray());
    }
    else
    {
        resize(980, 660);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Dialog buttons
    ////////////////////////////////////////////////////////////////////////////////////////////////

    dialogButtons_ = new QDialogButtonBox(this);
    dialogButtons_->addButton(QDialogButtonBox::Ok);
    dialogButtons_->addButton(QDialogButtonBox::Cancel);
    dialogButtons_->addButton(QDialogButtonBox::RestoreDefaults);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Tabs
    ////////////////////////////////////////////////////////////////////////////////////////////////

    setupUiDownscaleTab();
    setupUiPreprocessingTab();
    setupUiInitTab();
    setupUiAlgoTab();

    tabs_ = new QTabWidget(this);

    auto* tabBar = tabs_->tabBar();
    tabBar->setExpanding(false);
    tabBar->setUsesScrollButtons(false);
    tabBar->setElideMode(Qt::ElideNone);

    tabs_->addTab(downscalePage_, tr("Downscale"));
    tabs_->addTab(processPage_, tr("Processing"));
    tabs_->addTab(initPage_, tr("Initialization"));
    tabs_->addTab(algoPage_, tr("Algorithm"));

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Image preview
    ////////////////////////////////////////////////////////////////////////////////////////////////

    imageViewer_ = new ImageViewerWidget(this);
    auto interaction = std::make_unique<InteractionSet>();

    auto initBehavior = std::make_unique<InitializationBehavior>();
    initializationBehavior_ = initBehavior.get();

    interaction->addBehavior(std::move(initBehavior));

    imageViewer_->setInteraction(interaction.release());

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Layouts
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // Horizontal content: tabs | image
    QHBoxLayout* content_layout = new QHBoxLayout;
    content_layout->addWidget(tabs_);
    content_layout->addWidget(imageViewer_, 1); // l'image prend l'espace restant

    // Vertical root layout: content + buttons
    QVBoxLayout* root_layout = new QVBoxLayout;
    root_layout->addLayout(content_layout);
    root_layout->addWidget(dialogButtons_);

    setLayout(root_layout);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Controller
    ////////////////////////////////////////////////////////////////////////////////////////////////

    imageSettingsController_ = new ImageSettingsController(committedConfig_, this);

    updateUIFromConfig();
    setupConnections();
}

void SettingsWindow::setupUiAlgoTab()
{
    algoWidget_ = new AlgoSettingsWidget(committedConfig_.compute.algo, this);

    QVBoxLayout* algoLayout = new QVBoxLayout;
    algoLayout->addWidget(algoWidget_);

    algoPage_ = new QWidget;
    algoPage_->setLayout(algoLayout);
}

void SettingsWindow::setupUiInitTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialization tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* shape_groupbox = new QGroupBox(tr("Shape"));
    shape_groupbox->setFlat(true);

    rectangleRadio_ = new QRadioButton(tr("rectangle"));
    rectangleRadio_->setToolTip(tr("Middle mouse button: toggle rectangle / ellipse"));
    ellipseRadio_ = new QRadioButton(tr("ellipse"));
    ellipseRadio_->setToolTip(tr("Middle mouse button: toggle rectangle / ellipse"));

    rectangleRadio_->setChecked(false);
    ellipseRadio_->setChecked(true);

    QHBoxLayout* shape_layout = new QHBoxLayout;
    shape_layout->addWidget(rectangleRadio_);
    shape_layout->addWidget(ellipseRadio_);
    shape_groupbox->setLayout(shape_layout);

    ////////////////////////////////////////////

    QGroupBox* shape_size_groupbox = new QGroupBox(tr("Size"));
    shape_size_groupbox->setToolTip(tr("Ctrl + mouse wheel in the image to resize the shape"));

    widthShapeSpin_ = new QSpinBox;
    widthShapeSpin_->setSingleStep(10);
    widthShapeSpin_->setMinimum(0);
    widthShapeSpin_->setMaximum(200);
    widthShapeSpin_->setSuffix(tr(" % image width"));
    widthShapeSpin_->setValue(65);

    QFormLayout* width_spin_layout = new QFormLayout;
    width_spin_layout->addRow(tr("width ="), widthShapeSpin_);

    widthSlider_ = new QSlider(Qt::Horizontal, this);

    widthSlider_->setTickPosition(QSlider::TicksAbove);

    widthSlider_->setMinimum(0);
    widthSlider_->setMaximum(200);
    widthSlider_->setTickInterval(20);
    widthSlider_->setSingleStep(10);
    widthSlider_->setValue(65);

    heightShapeSpin_ = new QSpinBox;
    heightShapeSpin_->setSingleStep(10);
    heightShapeSpin_->setMinimum(0);
    heightShapeSpin_->setMaximum(200);
    heightShapeSpin_->setSuffix(tr((" % image height")));
    heightShapeSpin_->setValue(65);

    QFormLayout* height_spin_layout = new QFormLayout;
    height_spin_layout->addRow(tr("height ="), heightShapeSpin_);

    heightSlider_ = new QSlider(Qt::Horizontal, this);

    heightSlider_->setTickPosition(QSlider::TicksAbove);

    heightSlider_->setMinimum(0);
    heightSlider_->setMaximum(200);
    heightSlider_->setTickInterval(20);
    heightSlider_->setSingleStep(10);
    heightSlider_->setValue(65);

    QVBoxLayout* shape_size_layout = new QVBoxLayout;
    shape_size_layout->addLayout(width_spin_layout);
    shape_size_layout->addWidget(widthSlider_);
    shape_size_layout->addLayout(height_spin_layout);
    shape_size_layout->addWidget(heightSlider_);
    shape_size_groupbox->setLayout(shape_size_layout);

    ////////////////////////////////////////////

    QGroupBox* position_groupbox = new QGroupBox(tr("Position (x,y)"));
    position_groupbox->setToolTip(tr("Move the cursor in the image to set the position"));

    abscissaSpin_ = new QSpinBox;
    abscissaSpin_->setSingleStep(15);
    abscissaSpin_->setMinimum(-200);
    abscissaSpin_->setMaximum(200);
    abscissaSpin_->setSuffix(tr(" % image width"));
    abscissaSpin_->setValue(0);

    QFormLayout* abscissa_spin_layout = new QFormLayout;
    abscissa_spin_layout->addRow("x = Xo +", abscissaSpin_);

    abscissaSlider_ = new QSlider(Qt::Horizontal, this);

    abscissaSlider_->setTickPosition(QSlider::TicksAbove);

    abscissaSlider_->setMinimum(-75);
    abscissaSlider_->setMaximum(75);
    abscissaSlider_->setTickInterval(25);
    abscissaSlider_->setSingleStep(15);
    abscissaSlider_->setValue(0);

    ordinateSpin_ = new QSpinBox;
    ordinateSpin_->setSingleStep(15);
    ordinateSpin_->setMinimum(-200);
    ordinateSpin_->setMaximum(200);
    ordinateSpin_->setSuffix(tr(" % image height"));
    ordinateSpin_->setValue(0);

    QFormLayout* ordinate_spin_layout = new QFormLayout;
    ordinate_spin_layout->addRow("y = Yo +", ordinateSpin_);

    ordinateSlider_ = new QSlider(Qt::Horizontal, this);

    ordinateSlider_->setTickPosition(QSlider::TicksAbove);

    ordinateSlider_->setMinimum(-75);
    ordinateSlider_->setMaximum(75);
    ordinateSlider_->setTickInterval(25);
    ordinateSlider_->setSingleStep(15);
    ordinateSlider_->setValue(0);

    QVBoxLayout* position_layout = new QVBoxLayout;
    position_layout->addLayout(abscissa_spin_layout);
    position_layout->addWidget(abscissaSlider_);
    position_layout->addLayout(ordinate_spin_layout);
    position_layout->addWidget(ordinateSlider_);
    position_groupbox->setLayout(position_layout);

    ////////////////////////////////////////////

    QGroupBox* modify_groupbox = new QGroupBox(tr("Active contour modification"));
    modify_groupbox->setFlat(true);

    addButton_ = new QPushButton(tr("Add"));
    addButton_->setToolTip(tr("Left mouse button in the image: add the shape"));

    subtractButton_ = new QPushButton(tr("Subtract"));
    subtractButton_->setToolTip(tr("Right mouse button in the image: remove the shape"));

    clearButton_ = new QPushButton(tr("Clear"));

    QHBoxLayout* modify_layout = new QHBoxLayout;
    modify_layout->addWidget(clearButton_);
    modify_layout->addWidget(subtractButton_);
    modify_layout->addWidget(addButton_);
    modify_groupbox->setLayout(modify_layout);

    ////////////////////////////////////////////

    QVBoxLayout* initialization_layout = new QVBoxLayout;
    initialization_layout->addWidget(shape_groupbox);
    initialization_layout->addWidget(shape_size_groupbox);
    initialization_layout->addWidget(position_groupbox);
    initialization_layout->addWidget(modify_groupbox);
    initialization_layout->addStretch(1);

    initPage_ = new QWidget;
    initPage_->setLayout(initialization_layout);
}

void SettingsWindow::setupUiDownscaleTab()
{
    downscalePage_ = new QGroupBox(tr("Downscale"));
    downscalePage_->setCheckable(true);

    downscaleFactorCb_ = new QComboBox;
    downscaleFactorCb_->addItem("2", 2);
    downscaleFactorCb_->addItem("4", 4);

    auto* label = new QLabel(tr("Factor:"));

    auto* hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(downscaleFactorCb_);
    hbox->addStretch();

    auto* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    downscalePage_->setLayout(vbox);
}

void SettingsWindow::setupUiPreprocessingTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Preprocessing tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    gaussianNoiseGroupbox_ = new QGroupBox(tr("Gaussian Noise"));
    gaussianNoiseGroupbox_->setToolTip(
        tr("Additive Gaussian (white) noise applied independently to each pixel."));
    gaussianNoiseGroupbox_->setCheckable(true);
    gaussianNoiseGroupbox_->setChecked(false);
    gaussianNoiseStdSpin_ = new QDoubleSpinBox;
    gaussianNoiseStdSpin_->setSingleStep(5.0);
    gaussianNoiseStdSpin_->setMinimum(0.0);
    gaussianNoiseStdSpin_->setMaximum(80.0);
    gaussianNoiseStdSpin_->setToolTip(tr("standard deviation"));

    QFormLayout* gaussian_noise_layout = new QFormLayout;
    gaussian_noise_layout->addRow("σ =", gaussianNoiseStdSpin_);

    impulsiveNoiseGroupbox_ = new QGroupBox(tr("Impulsive Noise"));
    impulsiveNoiseGroupbox_->setToolTip(
        tr("Random pixel corruption. Grayscale: salt & pepper (black or white pixels). Color: each "
           "channel can be independently corrupted."));
    impulsiveNoiseGroupbox_->setCheckable(true);
    impulsiveNoiseGroupbox_->setChecked(false);
    impulsiveNoisePercentSpin_ = new QDoubleSpinBox;
    impulsiveNoisePercentSpin_->setSingleStep(1.0);
    impulsiveNoisePercentSpin_->setRange(0.0, 100.0);
    impulsiveNoisePercentSpin_->setSuffix(" %");
    impulsiveNoisePercentSpin_->setToolTip(tr("impulsional noise probability for each pixel"));

    QFormLayout* impulsiveNoiseLayout = new QFormLayout;
    impulsiveNoiseLayout->addRow(tr("d ="), impulsiveNoisePercentSpin_);

    speckleNoiseGroupbox_ = new QGroupBox(tr("Speckle Noise"));
    speckleNoiseGroupbox_->setToolTip(
        tr("Multiplicative noise applied to pixel intensities. Based on a gamma distribution "
           "(commonly used to model speckle in imaging systems)."));
    speckleNoiseGroupbox_->setCheckable(true);
    speckleNoiseGroupbox_->setChecked(false);
    speckleNoiseStdSpin_ = new QDoubleSpinBox;
    speckleNoiseStdSpin_->setSingleStep(0.01);
    speckleNoiseStdSpin_->setRange(0.0, 2.0);
    speckleNoiseStdSpin_->setToolTip(tr("standard deviation"));

    QFormLayout* speckleNoiseLayout = new QFormLayout;
    speckleNoiseLayout->addRow("σ =", speckleNoiseStdSpin_);

    gaussianNoiseGroupbox_->setLayout(gaussian_noise_layout);
    impulsiveNoiseGroupbox_->setLayout(impulsiveNoiseLayout);
    speckleNoiseGroupbox_->setLayout(speckleNoiseLayout);

    QVBoxLayout* noiseLayout = new QVBoxLayout;
    noiseLayout->addWidget(gaussianNoiseGroupbox_);
    noiseLayout->addWidget(impulsiveNoiseGroupbox_);
    noiseLayout->addWidget(speckleNoiseGroupbox_);
    noiseLayout->addStretch(1);

    ////////////////////////////////////////////

    meanGroupbox_ = new QGroupBox(tr("Mean filter"));
    meanGroupbox_->setCheckable(true);
    meanGroupbox_->setChecked(false);
    meanKernelSizeSpin_ = new KernelSizeSpinBox;
    meanKernelSizeSpin_->setSingleStep(2);
    meanKernelSizeSpin_->setMinimum(3);
    meanKernelSizeSpin_->setMaximum(499);

    QFormLayout* meanLayout = new QFormLayout;
    meanLayout->addRow(tr("kernel size ="), meanKernelSizeSpin_);

    gaussianGroupbox_ = new QGroupBox(tr("Gaussian filter"));
    gaussianGroupbox_->setCheckable(true);
    gaussianGroupbox_->setChecked(false);
    gaussianKernelSizeSpin_ = new KernelSizeSpinBox;
    gaussianKernelSizeSpin_->setSingleStep(2);
    gaussianKernelSizeSpin_->setMinimum(3);
    gaussianKernelSizeSpin_->setMaximum(499);
    gaussianSigmaSpin_ = new QDoubleSpinBox;
    gaussianSigmaSpin_->setSingleStep(0.1);
    gaussianSigmaSpin_->setMinimum(0.0);
    gaussianSigmaSpin_->setMaximum(1000000.0);
    gaussianSigmaSpin_->setToolTip(tr("standard deviation"));

    QFormLayout* gaussianLayout = new QFormLayout;
    gaussianLayout->addRow(tr("kernel size ="), gaussianKernelSizeSpin_);
    gaussianLayout->addRow("σ =", gaussianSigmaSpin_);

    ////////////////////////////////////////////

    medianGroupbox_ = new QGroupBox(tr("Median filter"));
    medianGroupbox_->setCheckable(true);
    medianGroupbox_->setChecked(false);
    medianKernelSizeSpin_ = new KernelSizeSpinBox;
    medianKernelSizeSpin_->setSingleStep(2);
    medianKernelSizeSpin_->setMinimum(3);
    medianKernelSizeSpin_->setMaximum(499);
    // klength_median_spin->setSuffix("²");
    medianDirectRadio_ = new QRadioButton("O(r log r)×O(n)");
    medianPerreaultRadio_ = new QRadioButton("O(1)×O(n)");

    QFormLayout* medianLayout = new QFormLayout;
    medianLayout->addRow(tr("kernel size ="), medianKernelSizeSpin_);
    medianLayout->addRow(tr("Sort-based median"), medianDirectRadio_);
    medianLayout->addRow(tr("Perreault's algorithm"), medianPerreaultRadio_);

    anisoGroupbox_ = new QGroupBox(tr("Perona-Malik anisotropic diffusion"));
    anisoGroupbox_->setCheckable(true);
    anisoGroupbox_->setChecked(false);
    anisoExpConductionRadio_ = new QRadioButton("g(∇I) = exp(-(|∇I|/κ)²)");
    anisoReciprocalConductionRadio_ = new QRadioButton("g(∇I) = 1/(1+(|∇I|/κ)²)");
    iterationFilterSpin_ = new QSpinBox;
    iterationFilterSpin_->setSingleStep(1);
    iterationFilterSpin_->setMinimum(0);
    iterationFilterSpin_->setMaximum(5000);
    lambdaSpin_ = new QDoubleSpinBox;
    lambdaSpin_->setSingleStep(0.01);
    lambdaSpin_->setMinimum(0.0);
    lambdaSpin_->setMaximum(1.0 / 4.0);
    kappaSpin_ = new QDoubleSpinBox;
    kappaSpin_->setSingleStep(1.0);
    kappaSpin_->setMinimum(0.0);
    kappaSpin_->setMaximum(10000.0);

    QFormLayout* anisoLayout = new QFormLayout;
    anisoLayout->addRow(tr("iterations ="), iterationFilterSpin_);
    anisoLayout->addRow("λ =", lambdaSpin_);
    anisoLayout->addRow("κ =", kappaSpin_);
    anisoLayout->addRow(tr("function 1 :"), anisoExpConductionRadio_);
    anisoLayout->addRow(tr("function 2 :"), anisoReciprocalConductionRadio_);

    ////////////////////////////////////////////

    openGroupbox_ = new QGroupBox(tr("Opening"));
    openGroupbox_->setCheckable(true);
    openGroupbox_->setChecked(false);
    openKernelSizeSpin_ = new KernelSizeSpinBox;
    openKernelSizeSpin_->setSingleStep(2);
    openKernelSizeSpin_->setMinimum(3);
    openKernelSizeSpin_->setMaximum(499);
    openKernelSizeSpin_->setToolTip(tr("the structuring element shape is a square and its "
                                       "origin is the center of the square"));

    QFormLayout* openLayout = new QFormLayout;
    openLayout->addRow(tr("SE size ="), openKernelSizeSpin_);

    closeGroupbox_ = new QGroupBox(tr("Closing"));
    closeGroupbox_->setCheckable(true);
    closeGroupbox_->setChecked(false);
    closeKernelSizeSpin_ = new KernelSizeSpinBox;
    closeKernelSizeSpin_->setSingleStep(2);
    closeKernelSizeSpin_->setMinimum(3);
    closeKernelSizeSpin_->setMaximum(499);
    closeKernelSizeSpin_->setToolTip(tr("the structuring element shape is a square and its "
                                        "origin is the center of the square"));

    QFormLayout* closeLayout = new QFormLayout;
    closeLayout->addRow(tr("SE size ="), closeKernelSizeSpin_);

    ////////////////////////////////////////////

    tophatGroupbox_ = new QGroupBox(tr("Top-hat transform"));
    tophatGroupbox_->setCheckable(true);
    tophatGroupbox_->setChecked(false);
    whitetophatRadio_ = new QRadioButton(tr("white top-hat"));
    whitetophatRadio_->setToolTip(tr("difference between the input image the opened"));
    blacktophatRadio_ = new QRadioButton(tr("black top-hat"));
    blacktophatRadio_->setToolTip(tr("difference between the closed and the input image"));
    tophatKernelSizeSpin_ = new KernelSizeSpinBox;
    tophatKernelSizeSpin_->setSingleStep(2);
    tophatKernelSizeSpin_->setMinimum(3);
    tophatKernelSizeSpin_->setMaximum(499);
    tophatKernelSizeSpin_->setToolTip(tr("the structuring element shape is a square and its "
                                         "origin is the center of the square"));

    QFormLayout* tophatLayout = new QFormLayout;
    tophatLayout->addRow(" ", whitetophatRadio_);
    tophatLayout->addRow(" ", blacktophatRadio_);
    tophatLayout->addRow(tr("SE size ="), tophatKernelSizeSpin_);

    algoGroupbox_ = new QGroupBox(tr("Algorithm"));
    naiveRadio_ = new QRadioButton(tr("naïve algorithm in O(r)×O(n)"));
    perreaultRadio_ = new QRadioButton(tr("Perreault's algorithm in O(1)×O(n)"));
    QVBoxLayout* algoLayout = new QVBoxLayout;
    algoLayout->addWidget(naiveRadio_);
    algoLayout->addWidget(perreaultRadio_);

    meanGroupbox_->setLayout(meanLayout);
    gaussianGroupbox_->setLayout(gaussianLayout);

    medianGroupbox_->setLayout(medianLayout);
    anisoGroupbox_->setLayout(anisoLayout);

    openGroupbox_->setLayout(openLayout);
    closeGroupbox_->setLayout(closeLayout);
    tophatGroupbox_->setLayout(tophatLayout);
    algoGroupbox_->setLayout(algoLayout);

    QVBoxLayout* filterLayoutLinear = new QVBoxLayout;
    filterLayoutLinear->addWidget(meanGroupbox_);
    filterLayoutLinear->addWidget(gaussianGroupbox_);

    QVBoxLayout* filterLayoutEdgePreserv = new QVBoxLayout;
    filterLayoutEdgePreserv->addWidget(medianGroupbox_);
    filterLayoutEdgePreserv->addWidget(anisoGroupbox_);

    QVBoxLayout* filterLayout = new QVBoxLayout;
    filterLayout->addWidget(openGroupbox_);
    filterLayout->addWidget(closeGroupbox_);
    filterLayout->addWidget(tophatGroupbox_);
    filterLayout->addWidget(algoGroupbox_);

    ////////////////////////////////////////////

    processInnerTabs_ = new QTabWidget(this);

    QWidget* pageNoise = new QWidget;
    QWidget* pageFilterIso = new QWidget;
    QWidget* pageFilterAnsio = new QWidget;
    QWidget* pageFilterMorpho = new QWidget;

    noiseLayout->addStretch(1);
    pageNoise->setLayout(noiseLayout);

    filterLayoutLinear->addStretch(1);
    pageFilterIso->setLayout(filterLayoutLinear);

    filterLayoutEdgePreserv->addStretch(1);
    pageFilterAnsio->setLayout(filterLayoutEdgePreserv);

    filterLayout->addStretch(1);
    pageFilterMorpho->setLayout(filterLayout);

    QTabWidget* filterTabs = new QTabWidget(this);
    filterTabs->addTab(pageFilterIso, tr("Linear"));
    filterTabs->addTab(pageFilterAnsio, tr("Edge preserving"));
    filterTabs->addTab(pageFilterMorpho, tr("Math morpho"));

    processInnerTabs_->addTab(pageNoise, tr("Noise generators"));
    processInnerTabs_->addTab(filterTabs, tr("Filters"));

    processPage_ = new QGroupBox(tr("Processing"));
    processPage_->setCheckable(true);
    processPage_->setChecked(false);

    timeFilt_ = new QLabel(this);
    timeFilt_->setText(tr("time = "));
    QGroupBox* timeFiltGroupbox = new QGroupBox(tr("Processing time"));
    QVBoxLayout* elapsedFiltLayout = new QVBoxLayout;
    elapsedFiltLayout->addWidget(timeFilt_);
    timeFiltGroupbox->setLayout(elapsedFiltLayout);

    QVBoxLayout* processLayout = new QVBoxLayout;
    processLayout->addWidget(processInnerTabs_);
    processLayout->addWidget(timeFiltGroupbox);
    processPage_->setLayout(processLayout);
}

void SettingsWindow::setupConnections()
{
    // connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_visu(int)) );

    connect(dialogButtons_, &QDialogButtonBox::accepted, this, &SettingsWindow::accept);

    connect(dialogButtons_, &QDialogButtonBox::rejected, this, &SettingsWindow::reject);

    auto* restoreBtn = dialogButtons_->button(QDialogButtonBox::RestoreDefaults);

    connect(restoreBtn, &QPushButton::clicked, this, &SettingsWindow::restoreDefaults);

    connect(widthSlider_, &QSlider::valueChanged, widthShapeSpin_, &QSpinBox::setValue);

    connect(widthShapeSpin_, QOverload<int>::of(&QSpinBox::valueChanged), widthSlider_,
            &QSlider::setValue);

    connect(heightSlider_, &QSlider::valueChanged, heightShapeSpin_, &QSpinBox::setValue);

    connect(heightShapeSpin_, QOverload<int>::of(&QSpinBox::valueChanged), heightSlider_,
            &QSlider::setValue);

    connect(ordinateSlider_, &QSlider::valueChanged, ordinateSpin_, &QSpinBox::setValue);

    connect(ordinateSpin_, QOverload<int>::of(&QSpinBox::valueChanged), ordinateSlider_,
            &QSlider::setValue);

    connect(abscissaSlider_, &QSlider::valueChanged, abscissaSpin_, &QSpinBox::setValue);

    connect(abscissaSpin_, QOverload<int>::of(&QSpinBox::valueChanged), abscissaSlider_,
            &QSlider::setValue);

    connect(downscalePage_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedDownscaleConfig_.hasDownscale = checked;
                notifyConfigEdited();

                onUiShapeChanged();
            });

    connect(downscaleFactorCb_, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                if (index < 0)
                    return;

                editedDownscaleConfig_.downscaleFactor =
                    downscaleFactorCb_->itemData(index).toInt();
                notifyConfigEdited();

                onUiShapeChanged();
            });

    connect(processPage_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.enabled = checked;
                notifyConfigEdited();
            });

    connect(gaussianNoiseGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_gaussian_noise = checked;
                notifyConfigEdited();
            });

    connect(gaussianNoiseStdSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.std_noise = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(impulsiveNoiseGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_salt_noise = checked;
                notifyConfigEdited();
            });

    connect(impulsiveNoisePercentSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.proba_noise = static_cast<float>(v) / 100.f;
                notifyConfigEdited();
            });

    connect(speckleNoiseGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_speckle_noise = checked;
                notifyConfigEdited();
            });

    connect(speckleNoiseStdSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.std_speckle_noise = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(meanGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_mean_filt = checked;
                notifyConfigEdited();
            });

    connect(meanKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_mean_length = v;
                notifyConfigEdited();
            });

    connect(gaussianGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_gaussian_filt = checked;
                notifyConfigEdited();
            });

    connect(gaussianKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_gaussian_length = v;
                notifyConfigEdited();
            });

    connect(gaussianSigmaSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.sigma = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(medianGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_median_filt = checked;
                notifyConfigEdited();
            });

    connect(medianKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_median_length = v;
                notifyConfigEdited();
            });

    connect(medianDirectRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_algo = false;
                notifyConfigEdited();
            });

    connect(medianPerreaultRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_algo = true;
                notifyConfigEdited();
            });

    connect(anisoGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_aniso_diff = checked;
                notifyConfigEdited();
            });

    connect(anisoExpConductionRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.aniso_option =
                    fluvel_ip::filter::ConductionFunction::Exponential;
                notifyConfigEdited();
            });

    connect(anisoReciprocalConductionRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.aniso_option =
                    fluvel_ip::filter::ConductionFunction::Rational;
                notifyConfigEdited();
            });

    connect(iterationFilterSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.max_itera = v;
                notifyConfigEdited();
            });

    connect(lambdaSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.lambda = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(kappaSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.kappa = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(openGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_open_filt = checked;
                notifyConfigEdited();
            });

    connect(openKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_open_length = v;
                notifyConfigEdited();
            });

    connect(closeGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_close_filt = checked;
                notifyConfigEdited();
            });

    connect(closeKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_close_length = v;
                notifyConfigEdited();
            });

    connect(tophatGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_top_hat_filt = checked;
                notifyConfigEdited();
            });

    connect(whitetophatRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.is_white_top_hat = true;
                notifyConfigEdited();
            });

    connect(blacktophatRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.is_white_top_hat = false;
                notifyConfigEdited();
            });

    connect(tophatKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_tophat_length = v;
                notifyConfigEdited();
            });

    connect(naiveRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_morpho = true;
                notifyConfigEdited();
            });

    connect(perreaultRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_morpho = false;
                notifyConfigEdited();
            });

    connect(imageSettingsController_, &ImageSettingsController::processingStarted, this,
            [this]()
            {
                timeFilt_->setText("...");
            });

    connect(imageSettingsController_, &ImageSettingsController::filterPipelineProcessed, this,
            [this](double elapsedSec)
            {
                timeFilt_->setText(tr("%1 s").arg(elapsedSec, 5, 'f', 2));
            });

    connect(addButton_, &QPushButton::clicked, this, &SettingsWindow::onAddShape);

    connect(subtractButton_, &QPushButton::clicked, this, &SettingsWindow::onSubtractShape);

    connect(clearButton_, &QPushButton::clicked, this, &SettingsWindow::onClearPhi);

    connect(imageSettingsController_, &ImageSettingsController::viewChanged, imageViewer_,
            &ImageViewerWidget::setImage);

    connect(algoWidget_, &AlgoSettingsWidget::connectivityChanged, imageSettingsController_,
            &ImageSettingsController::onConnectivityChanged);

    connect(rectangleRadio_, &QRadioButton::toggled, this, &SettingsWindow::onUiShapeChanged);
    connect(ellipseRadio_, &QRadioButton::toggled, this, &SettingsWindow::onUiShapeChanged);

    connect(widthShapeSpin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);
    connect(heightShapeSpin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);

    connect(abscissaSpin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);
    connect(ordinateSpin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);

    connect(this, &SettingsWindow::updateOverlay, imageSettingsController_,
            &ImageSettingsController::onUpdateOverlay);

    connect(initializationBehavior_, &InitializationBehavior::previewShapeRequested, this,
            &SettingsWindow::onPreviewShapeAt);

    connect(initializationBehavior_, &InitializationBehavior::addShapeRequested, this,
            &SettingsWindow::onAddShapeAt);

    connect(initializationBehavior_, &InitializationBehavior::removeShapeRequested, this,
            &SettingsWindow::onSubtractShapeAt);

    connect(initializationBehavior_, &InitializationBehavior::resizeShapeRequested, this,
            &SettingsWindow::onResizeShape);

    connect(initializationBehavior_, &InitializationBehavior::toggleShapeRequested, this,
            &SettingsWindow::onToggleShape);

    connect(tabs_, &QTabWidget::currentChanged, this, &SettingsWindow::onTabChanged);

    connect(this, &SettingsWindow::initializationModeChanged, imageSettingsController_,
            &ImageSettingsController::setInteractiveMode);
}

void SettingsWindow::onUiShapeChanged()
{
    emit updateOverlay(getUiShape());
}

UiShapeInfo SettingsWindow::getUiShape() const
{
    QPoint position{abscissaSpin_->value(), ordinateSpin_->value()};

    return getUiShapeAt(position);
}

UiShapeInfo SettingsWindow::getUiShapeAt(QPoint position) const
{
    UiShapeInfo uiShape;

    if (rectangleRadio_->isChecked())
        uiShape.shape = ShapeType::Rectangle;
    else if (ellipseRadio_->isChecked())
        uiShape.shape = ShapeType::Ellipse;
    else
        uiShape.shape = ShapeType::Rectangle;

    uiShape.width = widthShapeSpin_->value();
    uiShape.height = heightShapeSpin_->value();
    uiShape.x = position.x();
    uiShape.y = position.y();

    return uiShape;
}

void SettingsWindow::accept()
{
    auto& ds_config = committedConfig_.compute.downscale;

    ds_config.hasDownscale = downscalePage_->isChecked();
    ds_config.downscaleFactor = downscaleFactorCb_->currentData().toInt();

    auto& filt_config = committedConfig_.compute.processing;

    filt_config.enabled = processPage_->isChecked();
    filt_config.has_gaussian_noise = gaussianNoiseGroupbox_->isChecked();
    filt_config.std_noise = float(gaussianNoiseStdSpin_->value());
    filt_config.has_salt_noise = impulsiveNoiseGroupbox_->isChecked();
    filt_config.proba_noise = float(impulsiveNoisePercentSpin_->value()) / 100.f;
    filt_config.has_speckle_noise = speckleNoiseGroupbox_->isChecked();
    filt_config.std_speckle_noise = float(speckleNoiseStdSpin_->value());
    filt_config.has_median_filt = medianGroupbox_->isChecked();
    filt_config.kernel_median_length = medianKernelSizeSpin_->value();
    filt_config.has_O1_algo = medianPerreaultRadio_->isChecked();

    filt_config.has_mean_filt = meanGroupbox_->isChecked();
    filt_config.kernel_mean_length = meanKernelSizeSpin_->value();

    filt_config.has_gaussian_filt = gaussianGroupbox_->isChecked();
    filt_config.kernel_gaussian_length = gaussianKernelSizeSpin_->value();
    filt_config.sigma = float(gaussianSigmaSpin_->value());
    filt_config.has_aniso_diff = anisoGroupbox_->isChecked();

    filt_config.max_itera = iterationFilterSpin_->value();
    filt_config.lambda = float(lambdaSpin_->value());
    filt_config.kappa = float(kappaSpin_->value());
    if (anisoExpConductionRadio_->isChecked())
    {
        filt_config.aniso_option = fluvel_ip::filter::ConductionFunction::Exponential;
    }
    else if (anisoReciprocalConductionRadio_->isChecked())
    {
        filt_config.aniso_option = fluvel_ip::filter::ConductionFunction::Rational;
    }

    filt_config.has_open_filt = openGroupbox_->isChecked();
    filt_config.kernel_open_length = openKernelSizeSpin_->value();
    filt_config.has_close_filt = closeGroupbox_->isChecked();
    filt_config.kernel_close_length = closeKernelSizeSpin_->value();
    filt_config.has_top_hat_filt = tophatGroupbox_->isChecked();
    filt_config.is_white_top_hat = whitetophatRadio_->isChecked();
    filt_config.kernel_tophat_length = tophatKernelSizeSpin_->value();
    filt_config.has_O1_morpho = perreaultRadio_->isChecked();

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(committedConfig_.compute.initialPhi);
#endif

    committedConfig_.compute.initialPhi = imageSettingsController_->commit();

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(committedConfig_.compute.initialPhi);
#endif

    algoWidget_->accept();

    emit imageSessionSettingsAccepted(committedConfig_);

    QDialog::accept();
}

void SettingsWindow::updateUIFromConfig()
{
    QSignalBlocker blocker(this);

    const auto& ds_config = committedConfig_.compute.downscale;

    downscalePage_->setChecked(ds_config.hasDownscale);

    int index = downscaleFactorCb_->findData(ds_config.downscaleFactor);
    if (index >= 0)
        downscaleFactorCb_->setCurrentIndex(index);

    const auto& config_filter = committedConfig_.compute.processing;

    processPage_->setChecked(config_filter.enabled);
    gaussianNoiseGroupbox_->setChecked(config_filter.has_gaussian_noise);
    gaussianNoiseStdSpin_->setValue(double(config_filter.std_noise));
    impulsiveNoiseGroupbox_->setChecked(config_filter.has_salt_noise);
    impulsiveNoisePercentSpin_->setValue(double(100.f * config_filter.proba_noise));
    speckleNoiseGroupbox_->setChecked(config_filter.has_speckle_noise);
    speckleNoiseStdSpin_->setValue(double(config_filter.std_speckle_noise));

    medianGroupbox_->setChecked(config_filter.has_median_filt);
    medianKernelSizeSpin_->setValue(config_filter.kernel_median_length);

    if (config_filter.has_O1_algo)
    {
        medianPerreaultRadio_->setChecked(true);
        medianDirectRadio_->setChecked(false);
    }
    else
    {
        medianDirectRadio_->setChecked(true);
        medianPerreaultRadio_->setChecked(false);
    }

    meanGroupbox_->setChecked(config_filter.has_mean_filt);
    meanKernelSizeSpin_->setValue(config_filter.kernel_mean_length);

    gaussianGroupbox_->setChecked(config_filter.has_gaussian_filt);
    gaussianKernelSizeSpin_->setValue(config_filter.kernel_gaussian_length);
    gaussianSigmaSpin_->setValue(double(config_filter.sigma));

    anisoGroupbox_->setChecked(config_filter.has_aniso_diff);
    iterationFilterSpin_->setValue(config_filter.max_itera);
    lambdaSpin_->setValue(config_filter.lambda);
    kappaSpin_->setValue(config_filter.kappa);
    anisoExpConductionRadio_->setChecked(config_filter.aniso_option ==
                                         fluvel_ip::filter::ConductionFunction::Exponential);
    anisoReciprocalConductionRadio_->setChecked(config_filter.aniso_option ==
                                                fluvel_ip::filter::ConductionFunction::Rational);

    openGroupbox_->setChecked(config_filter.has_open_filt);
    openKernelSizeSpin_->setValue(config_filter.kernel_open_length);

    closeGroupbox_->setChecked(config_filter.has_close_filt);
    closeKernelSizeSpin_->setValue(config_filter.kernel_close_length);

    tophatGroupbox_->setChecked(config_filter.has_top_hat_filt);

    if (config_filter.is_white_top_hat)
    {
        whitetophatRadio_->setChecked(true);
    }
    else
    {
        blacktophatRadio_->setChecked(true);
    }
    tophatKernelSizeSpin_->setValue(config_filter.kernel_tophat_length);

    if (config_filter.has_O1_morpho)
    {
        perreaultRadio_->setChecked(true);
    }
    else
    {
        naiveRadio_->setChecked(true);
    }

    imageSettingsController_->revert();

    algoWidget_->reject();

    editedDownscaleConfig_ = committedConfig_.compute.downscale;
    editedProcessingConfig_ = committedConfig_.compute.processing;

    notifyConfigEdited();
}

void SettingsWindow::reject()
{
    updateUIFromConfig();

    QDialog::reject();
}

void SettingsWindow::restoreDefaults()
{
    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////
    /*
        connectivity_cb->setCurrentIndex( int(fluvel_ip::Connectivity::Four) );

        Na_spin->setValue( 30 );
        lambda_in_spin->setValue( 1 );
        lambda_out_spin->setValue( 1 );

        color_space_cb->setCurrentIndex( int(fluvel_ip::ColorSpaceOption::RGB) );

        alpha_spin->setValue( 1 );
        beta_spin->setValue( 1 );
        gamma_spin->setValue( 1 );

        downscale_factor_cb->setCurrentIndex( 1 );
        //has_temporal_smoothing_cb->setChecked( true );
        //cycles_nbr_sb->setValue( 3 );

        internalspeed_groupbox->setChecked( true );
        Ns_spin->setValue( 3 );
        disk_radius_spin->setValue( 2 );*/

    ///////////////////////////////////
    //        Preprocessing          //
    ///////////////////////////////////

    processPage_->setChecked(false);

    gaussianNoiseGroupbox_->setChecked(false);
    gaussianNoiseStdSpin_->setValue(20.0);

    impulsiveNoiseGroupbox_->setChecked(false);
    impulsiveNoisePercentSpin_->setValue(0.05);

    speckleNoiseGroupbox_->setChecked(false);
    speckleNoiseStdSpin_->setValue(0.16);

    medianGroupbox_->setChecked(false);
    medianKernelSizeSpin_->setValue(5);

    // has_O1_algo
    medianPerreaultRadio_->setChecked(true);

    meanGroupbox_->setChecked(false);
    meanKernelSizeSpin_->setValue(5);

    gaussianGroupbox_->setChecked(false);
    gaussianKernelSizeSpin_->setValue(5);
    gaussianSigmaSpin_->setValue(2.0);

    anisoGroupbox_->setChecked(false);
    // AnisoDiff::FUNCTION1
    anisoExpConductionRadio_->setChecked(true);
    anisoReciprocalConductionRadio_->setChecked(false);
    iterationFilterSpin_->setValue(10);
    lambdaSpin_->setValue(1.0 / 7.0);
    kappaSpin_->setValue(30.0);

    openGroupbox_->setChecked(false);
    openKernelSizeSpin_->setValue(5);

    closeGroupbox_->setChecked(false);
    closeKernelSizeSpin_->setValue(5);

    tophatGroupbox_->setChecked(false);
    whitetophatRadio_->setChecked(true);
    tophatKernelSizeSpin_->setValue(5);

    // config.has_O1_morpho
    perreaultRadio_->setChecked(true);

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    ellipseRadio_->setChecked(true);

    // 65% width and 65% height by default
    widthShapeSpin_->setValue(65);
    heightShapeSpin_->setValue(65);

    // centered by default
    abscissaSpin_->setValue(0);
    ordinateSpin_->setValue(0);

    editedDownscaleConfig_ = committedConfig_.compute.downscale;
    editedProcessingConfig_ = committedConfig_.compute.processing;

    notifyConfigEdited();
}

void SettingsWindow::onAddShape()
{
    if (imageSettingsController_)
        imageSettingsController_->addShape(getUiShape());
}

void SettingsWindow::onAddShapeAt(QPoint position)
{
    if (imageSettingsController_)
    {
        QPoint uiPosition = uiPositionFromView(position);
        imageSettingsController_->addShape(getUiShapeAt(uiPosition));
    }
}

void SettingsWindow::onSubtractShape()
{
    if (imageSettingsController_)
        imageSettingsController_->subtractShape(getUiShape());
}

void SettingsWindow::onSubtractShapeAt(QPoint position)
{
    if (imageSettingsController_)
    {
        QPoint uiPosition = uiPositionFromView(position);
        imageSettingsController_->subtractShape(getUiShapeAt(uiPosition));
    }
}

void SettingsWindow::onClearPhi()
{
    if (imageSettingsController_)
        imageSettingsController_->clearPhi();
}

void SettingsWindow::handleInputImageReady(const QImage& inputImage)
{
    if (!inputImage.isNull())
        imageSettingsController_->onInputImageReady(inputImage);
}

void SettingsWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    if (imageSettingsController_)
        imageSettingsController_->setViewVisible(true);

    emit updateOverlay(getUiShape());
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("ui_geometry/settings_window", saveGeometry());

    QDialog::closeEvent(event);
}

void SettingsWindow::notifyConfigEdited()
{
    if (imageSettingsController_)
        imageSettingsController_->updateEditedConfig(editedDownscaleConfig_,
                                                     editedProcessingConfig_);
}

void SettingsWindow::onPreviewShapeAt(QPoint position)
{
    QPoint uiPosition = uiPositionFromView(position);

    {
        QSignalBlocker b1(abscissaSpin_);
        QSignalBlocker b2(ordinateSpin_);

        QSignalBlocker b3(abscissaSlider_);
        QSignalBlocker b4(ordinateSlider_);

        abscissaSpin_->setValue(uiPosition.x());
        ordinateSpin_->setValue(uiPosition.y());

        abscissaSlider_->setValue(uiPosition.x());
        ordinateSlider_->setValue(uiPosition.y());
    }

    UiShapeInfo shape = getUiShapeAt(uiPosition);

    emit updateOverlay(shape);
}

void SettingsWindow::onResizeShape(int delta)
{
    wheelAccumulator_ += delta;

    const int stepUnit = 120;

    while (wheelAccumulator_ >= stepUnit)
    {
        widthShapeSpin_->stepUp();
        heightShapeSpin_->stepUp();
        wheelAccumulator_ -= stepUnit;
    }

    while (wheelAccumulator_ <= -stepUnit)
    {
        widthShapeSpin_->stepDown();
        heightShapeSpin_->stepDown();
        wheelAccumulator_ += stepUnit;
    }
}

void SettingsWindow::onToggleShape()
{
    if (rectangleRadio_->isChecked())
        ellipseRadio_->setChecked(true);
    else
        rectangleRadio_->setChecked(true);
}

QPoint SettingsWindow::uiPositionFromView(const QPoint& viewPosition) const
{
    if (imageViewer_->image().isNull())
        return QPoint(-1, -1);

    QSize viewSize = imageViewer_->image().size();

    float px = float(viewPosition.x()) / float(viewSize.width()) - 0.5f;
    float py = float(viewPosition.y()) / float(viewSize.height()) - 0.5f;

    int ux = int(px * 100.f);
    int uy = int(py * 100.f);

    return QPoint(ux, uy);
}

void SettingsWindow::onTabChanged(int index)
{
    constexpr int kInitializationTab = int(TabIndex::Initialization);
    emit initializationModeChanged(index == kInitializationTab);
}

} // namespace fluvel_app
