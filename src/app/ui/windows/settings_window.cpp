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

SettingsWindow::SettingsWindow(const ImageSessionSettings& config, QWidget* parent)
    : QDialog(parent)
    , committedConfig_(config)
    , editedDownscaleParams_(config.compute.downscale)
    , editedProcessingParams_(config.compute.processing)
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
    algoWidget_ = new AlgoSettingsWidget(committedConfig_.compute.contourConfig, this);

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

    ////////////////////////////////////////////

    medianGroupbox_ = new QGroupBox(tr("Median filter"));
    medianGroupbox_->setCheckable(true);
    medianGroupbox_->setChecked(false);
    medianKernelSizeSpin_ = new KernelSizeSpinBox;
    medianKernelSizeSpin_->setSingleStep(2);
    medianKernelSizeSpin_->setMinimum(3);
    medianKernelSizeSpin_->setMaximum(499);
    // klength_median_spin->setSuffix("²");

    QFormLayout* medianLayout = new QFormLayout;
    medianLayout->addRow(tr("kernel size ="), medianKernelSizeSpin_);

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

    meanGroupbox_->setLayout(meanLayout);

    medianGroupbox_->setLayout(medianLayout);
    anisoGroupbox_->setLayout(anisoLayout);

    openGroupbox_->setLayout(openLayout);
    closeGroupbox_->setLayout(closeLayout);
    tophatGroupbox_->setLayout(tophatLayout);

    QVBoxLayout* filterLayoutLinear = new QVBoxLayout;
    filterLayoutLinear->addWidget(meanGroupbox_);

    QVBoxLayout* filterLayoutEdgePreserv = new QVBoxLayout;
    filterLayoutEdgePreserv->addWidget(medianGroupbox_);
    filterLayoutEdgePreserv->addWidget(anisoGroupbox_);

    QVBoxLayout* filterLayout = new QVBoxLayout;
    filterLayout->addWidget(openGroupbox_);
    filterLayout->addWidget(closeGroupbox_);
    filterLayout->addWidget(tophatGroupbox_);

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
                editedDownscaleParams_.downscaleEnabled = checked;
                notifyConfigEdited();

                onUiShapeChanged();
            });

    connect(downscaleFactorCb_, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                if (index < 0)
                    return;

                editedDownscaleParams_.downscaleFactor =
                    downscaleFactorCb_->itemData(index).toInt();
                notifyConfigEdited();

                onUiShapeChanged();
            });

    connect(processPage_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.processingEnabled = checked;
                notifyConfigEdited();
            });

    connect(gaussianNoiseGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.gaussianNoiseEnabled = checked;
                notifyConfigEdited();
            });

    connect(gaussianNoiseStdSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingParams_.noiseStdDev = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(impulsiveNoiseGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.saltNoiseEnabled = checked;
                notifyConfigEdited();
            });

    connect(impulsiveNoisePercentSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingParams_.saltNoiseProbability = static_cast<float>(v) / 100.f;
                notifyConfigEdited();
            });

    connect(speckleNoiseGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.speckleNoiseEnabled = checked;
                notifyConfigEdited();
            });

    connect(speckleNoiseStdSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingParams_.speckleNoiseStdDev = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(meanGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.meanFilterEnabled = checked;
                notifyConfigEdited();
            });

    connect(meanKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingParams_.meanKernelSize = v;
                notifyConfigEdited();
            });

    connect(medianGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.medianFilterEnabled = checked;
                notifyConfigEdited();
            });

    connect(medianKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingParams_.medianKernelSize = v;
                notifyConfigEdited();
            });

    connect(anisoGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.anisotropicDiffusionEnabled = checked;
                notifyConfigEdited();
            });

    connect(anisoExpConductionRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingParams_.conductionFunction =
                    fluvel_ip::filter::ConductionFunction::Exponential;
                notifyConfigEdited();
            });

    connect(anisoReciprocalConductionRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingParams_.conductionFunction =
                    fluvel_ip::filter::ConductionFunction::Rational;
                notifyConfigEdited();
            });

    connect(iterationFilterSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingParams_.maxIterations = v;
                notifyConfigEdited();
            });

    connect(lambdaSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingParams_.lambda = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(kappaSpin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingParams_.kappa = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(openGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.openingEnabled = checked;
                notifyConfigEdited();
            });

    connect(openKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingParams_.openingKernelSize = v;
                notifyConfigEdited();
            });

    connect(closeGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.closingEnabled = checked;
                notifyConfigEdited();
            });

    connect(closeKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingParams_.closingKernelSize = v;
                notifyConfigEdited();
            });

    connect(tophatGroupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingParams_.topHatEnabled = checked;
                notifyConfigEdited();
            });

    connect(whitetophatRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingParams_.useWhiteTopHat = true;
                notifyConfigEdited();
            });

    connect(blacktophatRadio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingParams_.useWhiteTopHat = false;
                notifyConfigEdited();
            });

    connect(tophatKernelSizeSpin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingParams_.topHatKernelSize = v;
                notifyConfigEdited();
            });

    connect(imageSettingsController_, &ImageSettingsController::processingStarted, this,
            [this]()
            {
                timeFilt_->setText("...");
                timeFilt_->repaint();
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

    ds_config.downscaleEnabled = downscalePage_->isChecked();
    ds_config.downscaleFactor = downscaleFactorCb_->currentData().toInt();

    auto& filt_config = committedConfig_.compute.processing;

    filt_config.processingEnabled = processPage_->isChecked();
    filt_config.gaussianNoiseEnabled = gaussianNoiseGroupbox_->isChecked();
    filt_config.noiseStdDev = float(gaussianNoiseStdSpin_->value());
    filt_config.saltNoiseEnabled = impulsiveNoiseGroupbox_->isChecked();
    filt_config.saltNoiseProbability = float(impulsiveNoisePercentSpin_->value()) / 100.f;
    filt_config.speckleNoiseEnabled = speckleNoiseGroupbox_->isChecked();
    filt_config.speckleNoiseStdDev = float(speckleNoiseStdSpin_->value());
    filt_config.medianFilterEnabled = medianGroupbox_->isChecked();
    filt_config.medianKernelSize = medianKernelSizeSpin_->value();

    filt_config.meanFilterEnabled = meanGroupbox_->isChecked();
    filt_config.meanKernelSize = meanKernelSizeSpin_->value();

    filt_config.anisotropicDiffusionEnabled = anisoGroupbox_->isChecked();
    filt_config.maxIterations = iterationFilterSpin_->value();
    filt_config.lambda = float(lambdaSpin_->value());
    filt_config.kappa = float(kappaSpin_->value());
    if (anisoExpConductionRadio_->isChecked())
    {
        filt_config.conductionFunction = fluvel_ip::filter::ConductionFunction::Exponential;
    }
    else if (anisoReciprocalConductionRadio_->isChecked())
    {
        filt_config.conductionFunction = fluvel_ip::filter::ConductionFunction::Rational;
    }

    filt_config.openingEnabled = openGroupbox_->isChecked();
    filt_config.openingKernelSize = openKernelSizeSpin_->value();
    filt_config.closingEnabled = closeGroupbox_->isChecked();
    filt_config.closingKernelSize = closeKernelSizeSpin_->value();
    filt_config.topHatEnabled = tophatGroupbox_->isChecked();
    filt_config.useWhiteTopHat = whitetophatRadio_->isChecked();
    filt_config.topHatKernelSize = tophatKernelSizeSpin_->value();

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

    downscalePage_->setChecked(ds_config.downscaleEnabled);

    int index = downscaleFactorCb_->findData(ds_config.downscaleFactor);
    if (index >= 0)
        downscaleFactorCb_->setCurrentIndex(index);

    const auto& config_filter = committedConfig_.compute.processing;

    processPage_->setChecked(config_filter.processingEnabled);
    gaussianNoiseGroupbox_->setChecked(config_filter.gaussianNoiseEnabled);
    gaussianNoiseStdSpin_->setValue(double(config_filter.noiseStdDev));
    impulsiveNoiseGroupbox_->setChecked(config_filter.saltNoiseEnabled);
    impulsiveNoisePercentSpin_->setValue(double(100.f * config_filter.saltNoiseProbability));
    speckleNoiseGroupbox_->setChecked(config_filter.speckleNoiseEnabled);
    speckleNoiseStdSpin_->setValue(double(config_filter.speckleNoiseStdDev));

    medianGroupbox_->setChecked(config_filter.medianFilterEnabled);
    medianKernelSizeSpin_->setValue(config_filter.medianKernelSize);

    meanGroupbox_->setChecked(config_filter.meanFilterEnabled);
    meanKernelSizeSpin_->setValue(config_filter.meanKernelSize);

    anisoGroupbox_->setChecked(config_filter.anisotropicDiffusionEnabled);
    iterationFilterSpin_->setValue(config_filter.maxIterations);
    lambdaSpin_->setValue(config_filter.lambda);
    kappaSpin_->setValue(config_filter.kappa);
    anisoExpConductionRadio_->setChecked(config_filter.conductionFunction ==
                                         fluvel_ip::filter::ConductionFunction::Exponential);
    anisoReciprocalConductionRadio_->setChecked(config_filter.conductionFunction ==
                                                fluvel_ip::filter::ConductionFunction::Rational);

    openGroupbox_->setChecked(config_filter.openingEnabled);
    openKernelSizeSpin_->setValue(config_filter.openingKernelSize);

    closeGroupbox_->setChecked(config_filter.closingEnabled);
    closeKernelSizeSpin_->setValue(config_filter.closingKernelSize);

    tophatGroupbox_->setChecked(config_filter.topHatEnabled);

    if (config_filter.useWhiteTopHat)
    {
        whitetophatRadio_->setChecked(true);
    }
    else
    {
        blacktophatRadio_->setChecked(true);
    }
    tophatKernelSizeSpin_->setValue(config_filter.topHatKernelSize);

    imageSettingsController_->revert();

    algoWidget_->reject();

    editedDownscaleParams_ = committedConfig_.compute.downscale;
    editedProcessingParams_ = committedConfig_.compute.processing;

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

    meanGroupbox_->setChecked(false);
    meanKernelSizeSpin_->setValue(5);

    anisoGroupbox_->setChecked(false);
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

    editedDownscaleParams_ = committedConfig_.compute.downscale;
    editedProcessingParams_ = committedConfig_.compute.processing;

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
        imageSettingsController_->updateEditedConfig(editedDownscaleParams_,
                                                     editedProcessingParams_);
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
