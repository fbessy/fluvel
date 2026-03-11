// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "settings_window.hpp"

#include "filters.hpp"
#include "image_window.hpp"
#include "interaction_set.hpp"
#include "kernel_size_spinbox.hpp"

#include <QtWidgets>

#ifdef FLUVEL_DEBUG
#include "image_debug.hpp"
#endif

namespace fluvel_app
{

SettingsWindow::SettingsWindow(QWidget* parent, const ImageSessionSettings& config)
    : QDialog(parent)
    , config_(config)
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

    dial_buttons_ = new QDialogButtonBox(this);
    dial_buttons_->addButton(QDialogButtonBox::Ok);
    dial_buttons_->addButton(QDialogButtonBox::Cancel);
    dial_buttons_->addButton(QDialogButtonBox::RestoreDefaults);

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

    tabs_->addTab(downscale_page_, tr("Downscale"));
    tabs_->addTab(preprocess_page_, tr("Processing"));
    tabs_->addTab(init_page_, tr("Initialization"));
    tabs_->addTab(algo_page_, tr("Algorithm"));

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Image preview
    ////////////////////////////////////////////////////////////////////////////////////////////////

    settingsView_ = new ImageView(this);
    auto interaction = std::make_unique<InteractionSet>();

    auto initBehavior = std::make_unique<InitializationBehavior>();
    initializationBehavior_ = initBehavior.get();

    interaction->addBehavior(std::move(initBehavior));

    settingsView_->setInteraction(interaction.release());

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Layouts
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // Horizontal content: tabs | image
    QHBoxLayout* content_layout = new QHBoxLayout;
    content_layout->addWidget(tabs_);
    content_layout->addWidget(settingsView_, 1); // l'image prend l'espace restant

    // Vertical root layout: content + buttons
    QVBoxLayout* root_layout = new QVBoxLayout;
    root_layout->addLayout(content_layout);
    root_layout->addWidget(dial_buttons_);

    setLayout(root_layout);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Controller
    ////////////////////////////////////////////////////////////////////////////////////////////////

    imageSettingsController_ =
        new ImageSettingsController(config_.compute.downscale, config_.compute.processing,
                                    config_.compute.algo.connectivity, this);

    updateUIFromConfig();
    setupConnections();
}

void SettingsWindow::setupUiAlgoTab()
{
    algo_widget_ = new AlgoSettingsWidget(config_.compute.algo, this);

    QVBoxLayout* algo_layout = new QVBoxLayout;
    algo_layout->addWidget(algo_widget_);

    algo_page_ = new QWidget;
    algo_page_->setLayout(algo_layout);
}

void SettingsWindow::setupUiInitTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialization tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* shape_groupbox = new QGroupBox(tr("Shape"));
    shape_groupbox->setFlat(true);

    rectangle_radio_ = new QRadioButton(tr("rectangle"));
    rectangle_radio_->setToolTip(tr("Middle mouse button: toggle rectangle / ellipse"));
    ellipse_radio_ = new QRadioButton(tr("ellipse"));
    ellipse_radio_->setToolTip(tr("Middle mouse button: toggle rectangle / ellipse"));

    rectangle_radio_->setChecked(false);
    ellipse_radio_->setChecked(true);

    QHBoxLayout* shape_layout = new QHBoxLayout;
    shape_layout->addWidget(rectangle_radio_);
    shape_layout->addWidget(ellipse_radio_);
    shape_groupbox->setLayout(shape_layout);

    ////////////////////////////////////////////

    QGroupBox* shape_size_groupbox = new QGroupBox(tr("Size"));
    shape_size_groupbox->setToolTip(tr("Ctrl + mouse wheel in the image to resize the shape"));

    width_shape_spin_ = new QSpinBox;
    width_shape_spin_->setSingleStep(10);
    width_shape_spin_->setMinimum(0);
    width_shape_spin_->setMaximum(200);
    width_shape_spin_->setSuffix(tr(" % image width"));
    width_shape_spin_->setValue(65);

    QFormLayout* width_spin_layout = new QFormLayout;
    width_spin_layout->addRow(tr("width ="), width_shape_spin_);

    width_slider_ = new QSlider(Qt::Horizontal, this);

    width_slider_->setTickPosition(QSlider::TicksAbove);

    width_slider_->setMinimum(0);
    width_slider_->setMaximum(200);
    width_slider_->setTickInterval(20);
    width_slider_->setSingleStep(10);
    width_slider_->setValue(65);

    height_shape_spin_ = new QSpinBox;
    height_shape_spin_->setSingleStep(10);
    height_shape_spin_->setMinimum(0);
    height_shape_spin_->setMaximum(200);
    height_shape_spin_->setSuffix(tr((" % image height")));
    height_shape_spin_->setValue(65);

    QFormLayout* height_spin_layout = new QFormLayout;
    height_spin_layout->addRow(tr("height ="), height_shape_spin_);

    height_slider_ = new QSlider(Qt::Horizontal, this);

    height_slider_->setTickPosition(QSlider::TicksAbove);

    height_slider_->setMinimum(0);
    height_slider_->setMaximum(200);
    height_slider_->setTickInterval(20);
    height_slider_->setSingleStep(10);
    height_slider_->setValue(65);

    QVBoxLayout* shape_size_layout = new QVBoxLayout;
    shape_size_layout->addLayout(width_spin_layout);
    shape_size_layout->addWidget(width_slider_);
    shape_size_layout->addLayout(height_spin_layout);
    shape_size_layout->addWidget(height_slider_);
    shape_size_groupbox->setLayout(shape_size_layout);

    ////////////////////////////////////////////

    QGroupBox* position_groupbox = new QGroupBox(tr("Position (x,y)"));
    position_groupbox->setToolTip(tr("Move the cursor in the image to set the position"));

    abscissa_spin_ = new QSpinBox;
    abscissa_spin_->setSingleStep(15);
    abscissa_spin_->setMinimum(-200);
    abscissa_spin_->setMaximum(200);
    abscissa_spin_->setSuffix(tr(" % image width"));
    abscissa_spin_->setValue(0);

    QFormLayout* abscissa_spin_layout = new QFormLayout;
    abscissa_spin_layout->addRow("x = Xo +", abscissa_spin_);

    abscissa_slider_ = new QSlider(Qt::Horizontal, this);

    abscissa_slider_->setTickPosition(QSlider::TicksAbove);

    abscissa_slider_->setMinimum(-75);
    abscissa_slider_->setMaximum(75);
    abscissa_slider_->setTickInterval(25);
    abscissa_slider_->setSingleStep(15);
    abscissa_slider_->setValue(0);

    ordinate_spin_ = new QSpinBox;
    ordinate_spin_->setSingleStep(15);
    ordinate_spin_->setMinimum(-200);
    ordinate_spin_->setMaximum(200);
    ordinate_spin_->setSuffix(tr(" % image height"));
    ordinate_spin_->setValue(0);

    QFormLayout* ordinate_spin_layout = new QFormLayout;
    ordinate_spin_layout->addRow("y = Yo +", ordinate_spin_);

    ordinate_slider_ = new QSlider(Qt::Horizontal, this);

    ordinate_slider_->setTickPosition(QSlider::TicksAbove);

    ordinate_slider_->setMinimum(-75);
    ordinate_slider_->setMaximum(75);
    ordinate_slider_->setTickInterval(25);
    ordinate_slider_->setSingleStep(15);
    ordinate_slider_->setValue(0);

    QVBoxLayout* position_layout = new QVBoxLayout;
    position_layout->addLayout(abscissa_spin_layout);
    position_layout->addWidget(abscissa_slider_);
    position_layout->addLayout(ordinate_spin_layout);
    position_layout->addWidget(ordinate_slider_);
    position_groupbox->setLayout(position_layout);

    ////////////////////////////////////////////

    QGroupBox* modify_groupbox = new QGroupBox(tr("Active contour modification"));
    modify_groupbox->setFlat(true);

    add_button_ = new QPushButton(tr("Add"));
    add_button_->setToolTip(tr("Left mouse button in the image: add the shape"));

    subtract_button_ = new QPushButton(tr("Subtract"));
    subtract_button_->setToolTip(tr("Right mouse button in the image: remove the shape"));

    clear_button_ = new QPushButton(tr("Clear"));

    QHBoxLayout* modify_layout = new QHBoxLayout;
    modify_layout->addWidget(clear_button_);
    modify_layout->addWidget(subtract_button_);
    modify_layout->addWidget(add_button_);
    modify_groupbox->setLayout(modify_layout);

    ////////////////////////////////////////////

    QVBoxLayout* initialization_layout = new QVBoxLayout;
    initialization_layout->addWidget(shape_groupbox);
    initialization_layout->addWidget(shape_size_groupbox);
    initialization_layout->addWidget(position_groupbox);
    initialization_layout->addWidget(modify_groupbox);
    initialization_layout->addStretch(1);

    init_page_ = new QWidget;
    init_page_->setLayout(initialization_layout);
}

void SettingsWindow::setupUiDownscaleTab()
{
    downscale_page_ = new QGroupBox(tr("Downscale"));
    downscale_page_->setCheckable(true);

    downscale_factor_cb_ = new QComboBox;
    downscale_factor_cb_->addItem("2", 2);
    downscale_factor_cb_->addItem("4", 4);

    auto* label = new QLabel(tr("Factor:"));

    auto* hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(downscale_factor_cb_);
    hbox->addStretch();

    auto* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    downscale_page_->setLayout(vbox);
}

void SettingsWindow::setupUiPreprocessingTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Preprocessing tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    gaussian_noise_groupbox_ = new QGroupBox(tr("Gaussian white noise"));
    gaussian_noise_groupbox_->setCheckable(true);
    gaussian_noise_groupbox_->setChecked(false);
    std_noise_spin_ = new QDoubleSpinBox;
    std_noise_spin_->setSingleStep(5.0);
    std_noise_spin_->setMinimum(0.0);
    std_noise_spin_->setMaximum(80.0);
    std_noise_spin_->setToolTip(tr("standard deviation"));

    QFormLayout* gaussian_noise_layout = new QFormLayout;
    gaussian_noise_layout->addRow("σ =", std_noise_spin_);

    salt_noise_groupbox_ = new QGroupBox(tr("Impulsional noise (salt and pepper)"));
    salt_noise_groupbox_->setCheckable(true);
    salt_noise_groupbox_->setChecked(false);
    salt_percent_spin_ = new QDoubleSpinBox;
    salt_percent_spin_->setSingleStep(1.0);
    salt_percent_spin_->setRange(0.0, 100.0);
    salt_percent_spin_->setSuffix(" %");
    salt_percent_spin_->setToolTip(tr("impulsional noise probability for each pixel"));

    QFormLayout* salt_noise_layout = new QFormLayout;
    salt_noise_layout->addRow(tr("d ="), salt_percent_spin_);

    speckle_noise_groupbox_ = new QGroupBox(tr("Speckle noise"));
    speckle_noise_groupbox_->setCheckable(true);
    speckle_noise_groupbox_->setChecked(false);
    std_speckle_noise_spin_ = new QDoubleSpinBox;
    std_speckle_noise_spin_->setSingleStep(0.01);
    std_speckle_noise_spin_->setRange(0.0, 2.0);
    std_speckle_noise_spin_->setToolTip(tr("standard deviation"));

    QFormLayout* speckle_noise_layout = new QFormLayout;
    speckle_noise_layout->addRow("σ =", std_speckle_noise_spin_);

    gaussian_noise_groupbox_->setLayout(gaussian_noise_layout);
    salt_noise_groupbox_->setLayout(salt_noise_layout);
    speckle_noise_groupbox_->setLayout(speckle_noise_layout);

    QVBoxLayout* noise_layout = new QVBoxLayout;
    noise_layout->addWidget(gaussian_noise_groupbox_);
    noise_layout->addWidget(salt_noise_groupbox_);
    noise_layout->addWidget(speckle_noise_groupbox_);
    noise_layout->addStretch(1);

    ////////////////////////////////////////////

    mean_groupbox_ = new QGroupBox(tr("Mean filter"));
    mean_groupbox_->setCheckable(true);
    mean_groupbox_->setChecked(false);
    klength_mean_spin_ = new KernelSizeSpinBox;
    klength_mean_spin_->setSingleStep(2);
    klength_mean_spin_->setMinimum(3);
    klength_mean_spin_->setMaximum(499);

    QFormLayout* mean_layout = new QFormLayout;
    mean_layout->addRow(tr("kernel size ="), klength_mean_spin_);

    gaussian_groupbox_ = new QGroupBox(tr("Gaussian filter"));
    gaussian_groupbox_->setCheckable(true);
    gaussian_groupbox_->setChecked(false);
    klength_gaussian_spin_ = new KernelSizeSpinBox;
    klength_gaussian_spin_->setSingleStep(2);
    klength_gaussian_spin_->setMinimum(3);
    klength_gaussian_spin_->setMaximum(499);
    std_filter_spin_ = new QDoubleSpinBox;
    std_filter_spin_->setSingleStep(0.1);
    std_filter_spin_->setMinimum(0.0);
    std_filter_spin_->setMaximum(1000000.0);
    std_filter_spin_->setToolTip(tr("standard deviation"));

    QFormLayout* gaussian_layout = new QFormLayout;
    gaussian_layout->addRow(tr("kernel size ="), klength_gaussian_spin_);
    gaussian_layout->addRow("σ =", std_filter_spin_);

    ////////////////////////////////////////////

    median_groupbox_ = new QGroupBox(tr("Median filter"));
    median_groupbox_->setCheckable(true);
    median_groupbox_->setChecked(false);
    klength_median_spin_ = new KernelSizeSpinBox;
    klength_median_spin_->setSingleStep(2);
    klength_median_spin_->setMinimum(3);
    klength_median_spin_->setMaximum(499);
    // klength_median_spin->setSuffix("²");
    complex_sort_ = new QRadioButton("O(r log r)×O(n)");
    complex_perreault_ = new QRadioButton("O(1)×O(n)");

    QFormLayout* median_layout = new QFormLayout;
    median_layout->addRow(tr("kernel size ="), klength_median_spin_);
    median_layout->addRow(tr("Sort-based median"), complex_sort_);
    median_layout->addRow(tr("Perreault's algorithm"), complex_perreault_);

    aniso_groupbox_ = new QGroupBox(tr("Perona-Malik anisotropic diffusion"));
    aniso_groupbox_->setCheckable(true);
    aniso_groupbox_->setChecked(false);
    aniso1_radio_ = new QRadioButton("g(∇I) = exp(-(|∇I|/κ)²)");
    aniso2_radio_ = new QRadioButton("g(∇I) = 1/(1+(1+(|∇I|/κ)²)");
    iteration_filter_spin_ = new QSpinBox;
    iteration_filter_spin_->setSingleStep(1);
    iteration_filter_spin_->setMinimum(0);
    iteration_filter_spin_->setMaximum(5000);
    lambda_spin_ = new QDoubleSpinBox;
    lambda_spin_->setSingleStep(0.01);
    lambda_spin_->setMinimum(0.0);
    lambda_spin_->setMaximum(1.0 / 4.0);
    kappa_spin_ = new QDoubleSpinBox;
    kappa_spin_->setSingleStep(1.0);
    kappa_spin_->setMinimum(0.0);
    kappa_spin_->setMaximum(10000.0);

    QFormLayout* aniso_layout = new QFormLayout;
    aniso_layout->addRow(tr("iterations ="), iteration_filter_spin_);
    aniso_layout->addRow("λ =", lambda_spin_);
    aniso_layout->addRow("κ =", kappa_spin_);
    aniso_layout->addRow(tr("function 1 :"), aniso1_radio_);
    aniso_layout->addRow(tr("function 2 :"), aniso2_radio_);

    ////////////////////////////////////////////

    open_groupbox_ = new QGroupBox(tr("Opening"));
    open_groupbox_->setCheckable(true);
    open_groupbox_->setChecked(false);
    klength_open_spin_ = new KernelSizeSpinBox;
    klength_open_spin_->setSingleStep(2);
    klength_open_spin_->setMinimum(3);
    klength_open_spin_->setMaximum(499);
    klength_open_spin_->setToolTip(tr("the structuring element shape is a square and its "
                                      "origin is the center of the square"));

    QFormLayout* open_layout = new QFormLayout;
    open_layout->addRow(tr("SE size ="), klength_open_spin_);

    close_groupbox_ = new QGroupBox(tr("Closing"));
    close_groupbox_->setCheckable(true);
    close_groupbox_->setChecked(false);
    klength_close_spin_ = new KernelSizeSpinBox;
    klength_close_spin_->setSingleStep(2);
    klength_close_spin_->setMinimum(3);
    klength_close_spin_->setMaximum(499);
    klength_close_spin_->setToolTip(tr("the structuring element shape is a square and its "
                                       "origin is the center of the square"));

    QFormLayout* close_layout = new QFormLayout;
    close_layout->addRow(tr("SE size ="), klength_close_spin_);

    ////////////////////////////////////////////

    tophat_groupbox_ = new QGroupBox(tr("Top-hat transform"));
    tophat_groupbox_->setCheckable(true);
    tophat_groupbox_->setChecked(false);
    whitetophat_radio_ = new QRadioButton(tr("white top-hat"));
    whitetophat_radio_->setToolTip(tr("difference between the input image the opened"));
    blacktophat_radio_ = new QRadioButton(tr("black top-hat"));
    blacktophat_radio_->setToolTip(tr("difference between the closed and the input image"));
    klength_tophat_spin_ = new KernelSizeSpinBox;
    klength_tophat_spin_->setSingleStep(2);
    klength_tophat_spin_->setMinimum(3);
    klength_tophat_spin_->setMaximum(499);
    klength_tophat_spin_->setToolTip(tr("the structuring element shape is a square and its "
                                        "origin is the center of the square"));

    QFormLayout* tophat_layout = new QFormLayout;
    tophat_layout->addRow(" ", whitetophat_radio_);
    tophat_layout->addRow(" ", blacktophat_radio_);
    tophat_layout->addRow(tr("SE size ="), klength_tophat_spin_);

    algo_groupbox_ = new QGroupBox(tr("Algorithm"));
    complex1_morpho_radio_ = new QRadioButton(tr("naïve algorithm in O(r)×O(n)"));
    complex2_morpho_radio_ = new QRadioButton(tr("Perreault's algorithm in O(1)×O(n)"));
    QVBoxLayout* algo_layout = new QVBoxLayout;
    algo_layout->addWidget(complex1_morpho_radio_);
    algo_layout->addWidget(complex2_morpho_radio_);

    mean_groupbox_->setLayout(mean_layout);
    gaussian_groupbox_->setLayout(gaussian_layout);

    median_groupbox_->setLayout(median_layout);
    aniso_groupbox_->setLayout(aniso_layout);

    open_groupbox_->setLayout(open_layout);
    close_groupbox_->setLayout(close_layout);
    tophat_groupbox_->setLayout(tophat_layout);
    algo_groupbox_->setLayout(algo_layout);

    QVBoxLayout* filter_layout_linear = new QVBoxLayout;
    filter_layout_linear->addWidget(mean_groupbox_);
    filter_layout_linear->addWidget(gaussian_groupbox_);

    QVBoxLayout* filter_layout_edge_preserv = new QVBoxLayout;
    filter_layout_edge_preserv->addWidget(median_groupbox_);
    filter_layout_edge_preserv->addWidget(aniso_groupbox_);

    QVBoxLayout* filter_layout_mm = new QVBoxLayout;
    filter_layout_mm->addWidget(open_groupbox_);
    filter_layout_mm->addWidget(close_groupbox_);
    filter_layout_mm->addWidget(tophat_groupbox_);
    filter_layout_mm->addWidget(algo_groupbox_);

    ////////////////////////////////////////////

    preprocess_tabs_ = new QTabWidget(this);

    QWidget* page_noise = new QWidget;
    QWidget* page_filter_iso = new QWidget;
    QWidget* page_filter_ansio = new QWidget;
    QWidget* page_filter_morpho = new QWidget;

    noise_layout->addStretch(1);
    page_noise->setLayout(noise_layout);

    filter_layout_linear->addStretch(1);
    page_filter_iso->setLayout(filter_layout_linear);

    filter_layout_edge_preserv->addStretch(1);
    page_filter_ansio->setLayout(filter_layout_edge_preserv);

    filter_layout_mm->addStretch(1);
    page_filter_morpho->setLayout(filter_layout_mm);

    QTabWidget* filter_tabs = new QTabWidget(this);
    filter_tabs->addTab(page_filter_iso, tr("Linear"));
    filter_tabs->addTab(page_filter_ansio, tr("Edge preserving"));
    filter_tabs->addTab(page_filter_morpho, tr("Math morpho"));

    preprocess_tabs_->addTab(page_noise, tr("Noise generators"));
    preprocess_tabs_->addTab(filter_tabs, tr("Filters"));

    preprocess_page_ = new QGroupBox(tr("Processing"));
    preprocess_page_->setCheckable(true);
    preprocess_page_->setChecked(false);

    time_filt_ = new QLabel(this);
    time_filt_->setText(tr("time = "));
    QGroupBox* time_filt_groupbox = new QGroupBox(tr("Processing time"));
    QVBoxLayout* elapsed_filt_layout = new QVBoxLayout;
    elapsed_filt_layout->addWidget(time_filt_);
    time_filt_groupbox->setLayout(elapsed_filt_layout);

    QVBoxLayout* preprocess_layout = new QVBoxLayout;
    preprocess_layout->addWidget(preprocess_tabs_);
    preprocess_layout->addWidget(time_filt_groupbox);
    preprocess_page_->setLayout(preprocess_layout);
}

void SettingsWindow::setupConnections()
{
    // connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_visu(int)) );

    connect(dial_buttons_, &QDialogButtonBox::accepted, this, &SettingsWindow::accept);

    connect(dial_buttons_, &QDialogButtonBox::rejected, this, &SettingsWindow::reject);

    auto* restoreBtn = dial_buttons_->button(QDialogButtonBox::RestoreDefaults);

    connect(restoreBtn, &QPushButton::clicked, this, &SettingsWindow::default_settings);

    connect(width_slider_, &QSlider::valueChanged, width_shape_spin_, &QSpinBox::setValue);

    connect(width_shape_spin_, QOverload<int>::of(&QSpinBox::valueChanged), width_slider_,
            &QSlider::setValue);

    connect(height_slider_, &QSlider::valueChanged, height_shape_spin_, &QSpinBox::setValue);

    connect(height_shape_spin_, QOverload<int>::of(&QSpinBox::valueChanged), height_slider_,
            &QSlider::setValue);

    connect(ordinate_slider_, &QSlider::valueChanged, ordinate_spin_, &QSpinBox::setValue);

    connect(ordinate_spin_, QOverload<int>::of(&QSpinBox::valueChanged), ordinate_slider_,
            &QSlider::setValue);

    connect(abscissa_slider_, &QSlider::valueChanged, abscissa_spin_, &QSpinBox::setValue);

    connect(abscissa_spin_, QOverload<int>::of(&QSpinBox::valueChanged), abscissa_slider_,
            &QSlider::setValue);

    connect(downscale_page_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedDownscaleConfig_.hasDownscale = checked;
                notifyConfigEdited();

                onUiShapeChanged();
            });

    connect(downscale_factor_cb_, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                if (index < 0)
                    return;

                editedDownscaleConfig_.downscaleFactor =
                    downscale_factor_cb_->itemData(index).toInt();
                notifyConfigEdited();

                onUiShapeChanged();
            });

    connect(preprocess_page_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.enabled = checked;
                notifyConfigEdited();
            });

    connect(gaussian_noise_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_gaussian_noise = checked;
                notifyConfigEdited();
            });

    connect(std_noise_spin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.std_noise = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(salt_noise_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_salt_noise = checked;
                notifyConfigEdited();
            });

    connect(salt_percent_spin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.proba_noise = static_cast<float>(v) / 100.f;
                notifyConfigEdited();
            });

    connect(speckle_noise_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_speckle_noise = checked;
                notifyConfigEdited();
            });

    connect(std_speckle_noise_spin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.std_speckle_noise = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(mean_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_mean_filt = checked;
                notifyConfigEdited();
            });

    connect(klength_mean_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_mean_length = v;
                notifyConfigEdited();
            });

    connect(gaussian_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_gaussian_filt = checked;
                notifyConfigEdited();
            });

    connect(klength_gaussian_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_gaussian_length = v;
                notifyConfigEdited();
            });

    connect(std_filter_spin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.sigma = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(median_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_median_filt = checked;
                notifyConfigEdited();
            });

    connect(klength_median_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_median_length = v;
                notifyConfigEdited();
            });

    connect(complex_sort_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_algo = false;
                notifyConfigEdited();
            });

    connect(complex_perreault_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_algo = true;
                notifyConfigEdited();
            });

    connect(aniso_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_aniso_diff = checked;
                notifyConfigEdited();
            });

    connect(aniso1_radio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.aniso_option = fluvel_ip::AnisoDiff::FUNCTION1;
                notifyConfigEdited();
            });

    connect(aniso2_radio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.aniso_option = fluvel_ip::AnisoDiff::FUNCTION2;
                notifyConfigEdited();
            });

    connect(iteration_filter_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.max_itera = v;
                notifyConfigEdited();
            });

    connect(lambda_spin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.lambda = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(kappa_spin_, &QDoubleSpinBox::valueChanged, this,
            [this](double v)
            {
                editedProcessingConfig_.kappa = static_cast<float>(v);
                notifyConfigEdited();
            });

    connect(open_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_open_filt = checked;
                notifyConfigEdited();
            });

    connect(klength_open_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_open_length = v;
                notifyConfigEdited();
            });

    connect(close_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_close_filt = checked;
                notifyConfigEdited();
            });

    connect(klength_close_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_close_length = v;
                notifyConfigEdited();
            });

    connect(tophat_groupbox_, &QGroupBox::clicked, this,
            [this](bool checked)
            {
                editedProcessingConfig_.has_top_hat_filt = checked;
                notifyConfigEdited();
            });

    connect(whitetophat_radio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.is_white_top_hat = true;
                notifyConfigEdited();
            });

    connect(blacktophat_radio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.is_white_top_hat = false;
                notifyConfigEdited();
            });

    connect(klength_tophat_spin_, &QSpinBox::valueChanged, this,
            [this](int v)
            {
                editedProcessingConfig_.kernel_tophat_length = v;
                notifyConfigEdited();
            });

    connect(complex1_morpho_radio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_morpho = true;
                notifyConfigEdited();
            });

    connect(complex2_morpho_radio_, &QRadioButton::clicked, this,
            [this]()
            {
                editedProcessingConfig_.has_O1_morpho = false;
                notifyConfigEdited();
            });

    connect(imageSettingsController_, &ImageSettingsController::processingStarted, this,
            [this]()
            {
                time_filt_->setText("...");
            });

    connect(imageSettingsController_, &ImageSettingsController::filterPipelineProcessed, this,
            [this](double elapsedSec)
            {
                time_filt_->setText(tr("%1 s").arg(elapsedSec, 5, 'f', 2));
            });

    connect(add_button_, &QPushButton::clicked, this, &SettingsWindow::onAddShape);

    connect(subtract_button_, &QPushButton::clicked, this, &SettingsWindow::onSubtractShape);

    connect(clear_button_, &QPushButton::clicked, this, &SettingsWindow::onClearPhi);

    connect(imageSettingsController_, &ImageSettingsController::viewChanged, settingsView_,
            &ImageView::setImage);

    connect(algo_widget_, &AlgoSettingsWidget::connectivityChanged, imageSettingsController_,
            &ImageSettingsController::onConnectivityChanged);

    connect(rectangle_radio_, &QRadioButton::toggled, this, &SettingsWindow::onUiShapeChanged);
    connect(ellipse_radio_, &QRadioButton::toggled, this, &SettingsWindow::onUiShapeChanged);

    connect(width_shape_spin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);
    connect(height_shape_spin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);

    connect(abscissa_spin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);
    connect(ordinate_spin_, &QSpinBox::valueChanged, this, &SettingsWindow::onUiShapeChanged);

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
    QPoint position{abscissa_spin_->value(), ordinate_spin_->value()};

    return getUiShapeAt(position);
}

UiShapeInfo SettingsWindow::getUiShapeAt(QPoint position) const
{
    UiShapeInfo uiShape;

    if (rectangle_radio_->isChecked())
        uiShape.shape = ShapeType::Rectangle;
    else if (ellipse_radio_->isChecked())
        uiShape.shape = ShapeType::Ellipse;
    else
        uiShape.shape = ShapeType::Rectangle;

    uiShape.width = width_shape_spin_->value();
    uiShape.height = height_shape_spin_->value();
    uiShape.x = position.x();
    uiShape.y = position.y();

    return uiShape;
}

void SettingsWindow::accept()
{
    auto& ds_config = config_.compute.downscale;

    ds_config.hasDownscale = downscale_page_->isChecked();
    ds_config.downscaleFactor = downscale_factor_cb_->currentData().toInt();

    auto& filt_config = config_.compute.processing;

    filt_config.enabled = preprocess_page_->isChecked();
    filt_config.has_gaussian_noise = gaussian_noise_groupbox_->isChecked();
    filt_config.std_noise = float(std_noise_spin_->value());
    filt_config.has_salt_noise = salt_noise_groupbox_->isChecked();
    filt_config.proba_noise = float(salt_percent_spin_->value()) / 100.f;
    filt_config.has_speckle_noise = speckle_noise_groupbox_->isChecked();
    filt_config.std_speckle_noise = float(std_speckle_noise_spin_->value());
    filt_config.has_median_filt = median_groupbox_->isChecked();
    filt_config.kernel_median_length = klength_median_spin_->value();
    filt_config.has_O1_algo = complex_perreault_->isChecked();

    filt_config.has_mean_filt = mean_groupbox_->isChecked();
    filt_config.kernel_mean_length = klength_mean_spin_->value();

    filt_config.has_gaussian_filt = gaussian_groupbox_->isChecked();
    filt_config.kernel_gaussian_length = klength_gaussian_spin_->value();
    filt_config.sigma = float(std_filter_spin_->value());
    filt_config.has_aniso_diff = aniso_groupbox_->isChecked();

    filt_config.max_itera = iteration_filter_spin_->value();
    filt_config.lambda = float(lambda_spin_->value());
    filt_config.kappa = float(kappa_spin_->value());
    if (aniso1_radio_->isChecked())
    {
        filt_config.aniso_option = fluvel_ip::AnisoDiff::FUNCTION1;
    }
    else if (aniso2_radio_->isChecked())
    {
        filt_config.aniso_option = fluvel_ip::AnisoDiff::FUNCTION2;
    }

    filt_config.has_open_filt = open_groupbox_->isChecked();
    filt_config.kernel_open_length = klength_open_spin_->value();
    filt_config.has_close_filt = close_groupbox_->isChecked();
    filt_config.kernel_close_length = klength_close_spin_->value();
    filt_config.has_top_hat_filt = tophat_groupbox_->isChecked();
    filt_config.is_white_top_hat = whitetophat_radio_->isChecked();
    filt_config.kernel_tophat_length = klength_tophat_spin_->value();
    filt_config.has_O1_morpho = complex2_morpho_radio_->isChecked();

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(config_.compute.initialPhi);
#endif

    config_.compute.initialPhi = imageSettingsController_->commit();

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(config_.compute.initialPhi);
#endif

    algo_widget_->accept();

    emit settingsAccepted(config_);

    QDialog::accept();
}

void SettingsWindow::updateUIFromConfig()
{
    QSignalBlocker blocker(this);

    const auto& ds_config = config_.compute.downscale;

    downscale_page_->setChecked(ds_config.hasDownscale);

    int index = downscale_factor_cb_->findData(ds_config.downscaleFactor);
    if (index >= 0)
        downscale_factor_cb_->setCurrentIndex(index);

    const auto& config_filter = config_.compute.processing;

    preprocess_page_->setChecked(config_filter.enabled);
    gaussian_noise_groupbox_->setChecked(config_filter.has_gaussian_noise);
    std_noise_spin_->setValue(double(config_filter.std_noise));
    salt_noise_groupbox_->setChecked(config_filter.has_salt_noise);
    salt_percent_spin_->setValue(double(100.f * config_filter.proba_noise));
    speckle_noise_groupbox_->setChecked(config_filter.has_speckle_noise);
    std_speckle_noise_spin_->setValue(double(config_filter.std_speckle_noise));

    median_groupbox_->setChecked(config_filter.has_median_filt);
    klength_median_spin_->setValue(config_filter.kernel_median_length);

    if (config_filter.has_O1_algo)
    {
        complex_perreault_->setChecked(true);
        complex_sort_->setChecked(false);
    }
    else
    {
        complex_sort_->setChecked(true);
        complex_perreault_->setChecked(false);
    }

    mean_groupbox_->setChecked(config_filter.has_mean_filt);
    klength_mean_spin_->setValue(config_filter.kernel_mean_length);

    gaussian_groupbox_->setChecked(config_filter.has_gaussian_filt);
    klength_gaussian_spin_->setValue(config_filter.kernel_gaussian_length);
    std_filter_spin_->setValue(double(config_filter.sigma));

    aniso_groupbox_->setChecked(config_filter.has_aniso_diff);
    iteration_filter_spin_->setValue(config_filter.max_itera);
    lambda_spin_->setValue(double(config_filter.lambda));
    kappa_spin_->setValue(double(config_filter.kappa));
    aniso1_radio_->setChecked(config_filter.aniso_option == fluvel_ip::AnisoDiff::FUNCTION1);
    aniso2_radio_->setChecked(config_filter.aniso_option == fluvel_ip::AnisoDiff::FUNCTION2);

    open_groupbox_->setChecked(config_filter.has_open_filt);
    klength_open_spin_->setValue(config_filter.kernel_open_length);

    close_groupbox_->setChecked(config_filter.has_close_filt);
    klength_close_spin_->setValue(config_filter.kernel_close_length);

    tophat_groupbox_->setChecked(config_filter.has_top_hat_filt);

    if (config_filter.is_white_top_hat)
    {
        whitetophat_radio_->setChecked(true);
    }
    else
    {
        blacktophat_radio_->setChecked(true);
    }
    klength_tophat_spin_->setValue(config_filter.kernel_tophat_length);

    if (config_filter.has_O1_morpho)
    {
        complex2_morpho_radio_->setChecked(true);
    }
    else
    {
        complex1_morpho_radio_->setChecked(true);
    }

    imageSettingsController_->revert();

    algo_widget_->reject();
}

void SettingsWindow::reject()
{
    updateUIFromConfig();

    QDialog::reject();
}

void SettingsWindow::default_settings()
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

    preprocess_page_->setChecked(false);

    gaussian_noise_groupbox_->setChecked(false);
    std_noise_spin_->setValue(20.0);

    salt_noise_groupbox_->setChecked(false);
    salt_percent_spin_->setValue(0.05);

    speckle_noise_groupbox_->setChecked(false);
    std_speckle_noise_spin_->setValue(0.16);

    median_groupbox_->setChecked(false);
    klength_median_spin_->setValue(5);

    // has_O1_algo
    complex_perreault_->setChecked(true);

    mean_groupbox_->setChecked(false);
    klength_mean_spin_->setValue(5);

    gaussian_groupbox_->setChecked(false);
    klength_gaussian_spin_->setValue(5);
    std_filter_spin_->setValue(2.0);

    aniso_groupbox_->setChecked(false);
    // AnisoDiff::FUNCTION1
    aniso1_radio_->setChecked(true);
    aniso2_radio_->setChecked(false);
    iteration_filter_spin_->setValue(10);
    lambda_spin_->setValue(1.0 / 7.0);
    kappa_spin_->setValue(30.0);

    open_groupbox_->setChecked(false);
    klength_open_spin_->setValue(5);

    close_groupbox_->setChecked(false);
    klength_close_spin_->setValue(5);

    tophat_groupbox_->setChecked(false);
    whitetophat_radio_->setChecked(true);
    klength_tophat_spin_->setValue(5);

    // config.has_O1_morpho
    complex2_morpho_radio_->setChecked(true);

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    ellipse_radio_->setChecked(true);

    // 65% width and 65% height by default
    width_shape_spin_->setValue(65);
    height_shape_spin_->setValue(65);

    // centered by default
    abscissa_spin_->setValue(0);
    ordinate_spin_->setValue(0);
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

void SettingsWindow::onInputImageReady(const QImage& inputImage)
{
    if (!inputImage.isNull())
        imageSettingsController_->onInputImageReady(inputImage);
}

void SettingsWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

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

    UiShapeInfo shape = getUiShapeAt(uiPosition);

    emit updateOverlay(shape);
}

void SettingsWindow::onResizeShape(int delta)
{
    wheelAccumulator_ += delta;

    const int stepUnit = 120;

    while (wheelAccumulator_ >= stepUnit)
    {
        width_shape_spin_->stepUp();
        height_shape_spin_->stepUp();
        wheelAccumulator_ -= stepUnit;
    }

    while (wheelAccumulator_ <= -stepUnit)
    {
        width_shape_spin_->stepDown();
        height_shape_spin_->stepDown();
        wheelAccumulator_ += stepUnit;
    }

    emit updateOverlay(getUiShape());
}

void SettingsWindow::onToggleShape()
{
    if (rectangle_radio_->isChecked())
        ellipse_radio_->setChecked(true);
    else
        rectangle_radio_->setChecked(true);
}

QPoint SettingsWindow::uiPositionFromView(const QPoint& viewPosition) const
{
    if (settingsView_->image().isNull())
        return QPoint(-1, -1);

    QSize viewSize = settingsView_->image().size();

    float px = float(viewPosition.x()) / float(viewSize.width()) - 0.5f;
    float py = float(viewPosition.y()) / float(viewSize.height()) - 0.5f;

    int ux = int(px * 100.f);
    int uy = int(py * 100.f);

    return QPoint(ux, uy);
}

void SettingsWindow::onTabChanged(int index)
{
    constexpr int kInitializationTab = 2;
    emit initializationModeChanged(index == kInitializationTab);
}

} // namespace fluvel_app
