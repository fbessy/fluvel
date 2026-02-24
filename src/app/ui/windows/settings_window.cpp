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

#include "settings_window.hpp"
#include "contour_rendering_qimage.hpp"
#include "application_settings.hpp"

#include "image_window.hpp"
#include "filters.hpp"
#include "kernel_size_spinbox.hpp"
#include "boundary_builder.hpp"
#include "active_contour.hpp"
#include "shape_overlay_renderer.hpp"
#include "phi_view_model.hpp"

#include "interaction_set.hpp"
#include "zoom_behavior.hpp"

#include <QtWidgets>

#include <stack>
#include <ctime>         // for std::clock_t, std::clock() and CLOCKS_PER_SEC
#include <cstring>       // for std::memcpy

namespace ofeli_app
{

SettingsWindow::SettingsWindow(QWidget* parent) :
    QDialog(parent)
    
{
    setWindowTitle(tr("Image session settings"));

    QSettings settings;

    const auto geo = settings.value("Settings/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    dial_buttons_ = new QDialogButtonBox(this);
    dial_buttons_->addButton(QDialogButtonBox::Ok);
    dial_buttons_->addButton(QDialogButtonBox::Cancel);
    dial_buttons_->addButton(QDialogButtonBox::RestoreDefaults);

    setupUiDownscaleTab();
    setupUiPreprocessingTab();
    setupUiInitTab();
    setupUiAlgoTab();

    tabs_ = new QTabWidget(this);

    auto *tabBar = tabs_->tabBar();

    tabBar->setExpanding(false);
    tabBar->setUsesScrollButtons(false);
    tabBar->setElideMode(Qt::ElideNone);

    tabs_->addTab( downscale_page_, tr("Downscale") );
    tabs_->addTab( preprocess_page_, tr("Processing") );
    tabs_->addTab( init_page_, tr("Initialization") );
    tabs_->addTab( algo_page_, tr("Algorithm") );


    settingsView_ = new ImageView(this);

    //auto interaction = std::make_unique<InteractionSet>();
    //interaction->addBehavior(std::make_unique<ZoomBehavior>());

    //settingsView->setInteraction(interaction.release());

    QGridLayout *settings_grid = new QGridLayout;
    settings_grid->addWidget(tabs_,0,0);
    settings_grid->addWidget(settingsView_,0,1);

    settings_grid->addWidget(dial_buttons_,1,1);

    settings_grid->setColumnStretch(1,1);
    setLayout(settings_grid);

    imageSettingsController_ = new ImageSettingsController(this);

    updateUIFromConfig();
    setupConnections();
}

void SettingsWindow::setupUiAlgoTab()
{
    algo_widget_ = new AlgoSettingsWidget(this,
                                         Session::Image);

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
    rectangle_radio_->setToolTip(tr("or click on the middle mouse button when the cursor is in the image"));
    ellipse_radio_ = new QRadioButton(tr("ellipse"));
    ellipse_radio_->setToolTip(tr("or click on the middle mouse button when the cursor is in the image"));

    rectangle_radio_->setChecked(false);
    ellipse_radio_->setChecked(true);

    QHBoxLayout* shape_layout = new QHBoxLayout;
    shape_layout->addWidget(rectangle_radio_);
    shape_layout->addWidget(ellipse_radio_);
    shape_groupbox->setLayout(shape_layout);

    ////////////////////////////////////////////

    QGroupBox* shape_size_groupbox = new QGroupBox(tr("Size"));
    shape_size_groupbox ->setToolTip(tr("or roll the mouse wheel when the cursor is in the image"));

    width_shape_spin_ = new QSpinBox;
    width_shape_spin_->setSingleStep(15);
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
    width_slider_->setTickInterval(25);
    width_slider_->setSingleStep(15);
    width_slider_->setValue(65);



    height_shape_spin_ = new QSpinBox;
    height_shape_spin_->setSingleStep(15);
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
    height_slider_->setTickInterval(25);
    height_slider_->setSingleStep(15);
    height_slider_->setValue(65);



    QVBoxLayout* shape_size_layout = new QVBoxLayout;
    shape_size_layout->addLayout(width_spin_layout);
    shape_size_layout->addWidget(width_slider_);
    shape_size_layout->addLayout(height_spin_layout);
    shape_size_layout->addWidget(height_slider_);
    shape_size_groupbox->setLayout(shape_size_layout);

    ////////////////////////////////////////////

    QGroupBox* position_groupbox = new QGroupBox(tr("Position (x,y)"));
    position_groupbox->setToolTip(tr("or move the mouse cursor in the image"));

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
    add_button_->setToolTip(tr("or click on the left mouse button when the cursor is in the image"));

    subtract_button_ = new QPushButton(tr("Subtract"));
    subtract_button_->setToolTip(tr("or click on the right mouse button when the cursor is in the image"));

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

    auto *label = new QLabel(tr("Factor:"));

    auto *hbox = new QHBoxLayout;
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

    //is_downscale_cb = new QCheckBox("Downscale :");
    //is_downscale_cb->setChecked(false);

    gaussian_noise_groupbox_ = new QGroupBox(tr("Gaussian white noise"));
    gaussian_noise_groupbox_->setCheckable(true);
    gaussian_noise_groupbox_->setChecked(false);
    std_noise_spin_ = new QDoubleSpinBox;
    std_noise_spin_->setSingleStep(5.0);
    std_noise_spin_->setMinimum(0.0);
    std_noise_spin_->setMaximum(10000.0);
    std_noise_spin_->setToolTip(tr("standard deviation"));


    QFormLayout* gaussian_noise_layout = new QFormLayout;
    gaussian_noise_layout->addRow("σ =", std_noise_spin_);

    salt_noise_groupbox_ = new QGroupBox(tr("Impulsional noise (salt and pepper)"));
    salt_noise_groupbox_->setCheckable(true);
    salt_noise_groupbox_->setChecked(false);
    proba_noise_spin_ = new QDoubleSpinBox;
    proba_noise_spin_->setSingleStep(1.0);
    proba_noise_spin_->setMinimum(0.0);
    proba_noise_spin_->setMaximum(100.0);
    proba_noise_spin_->setSuffix(" %");
    proba_noise_spin_->setToolTip(tr("impulsional noise probability for each pixel"));


    QFormLayout* salt_noise_layout = new QFormLayout;
    salt_noise_layout->addRow(tr("d ="), proba_noise_spin_);

    speckle_noise_groupbox_ = new QGroupBox(tr("Speckle noise"));
    speckle_noise_groupbox_->setCheckable(true);
    speckle_noise_groupbox_->setChecked(false);
    std_speckle_noise_spin_ = new QDoubleSpinBox;
    std_speckle_noise_spin_->setSingleStep(0.02);
    std_speckle_noise_spin_->setMinimum(0.0);
    std_speckle_noise_spin_->setMaximum(1000.0);
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
    //klength_median_spin->setSuffix("²");
    complex_radio1_= new QRadioButton("O(r log r)×O(n)");
    complex_radio2_= new QRadioButton("O(1)×O(n)");

    QFormLayout* median_layout = new QFormLayout;
    median_layout->addRow(tr("kernel size ="), klength_median_spin_);
    median_layout->addRow(tr("quick sort algorithm"), complex_radio1_);
    median_layout->addRow(tr("Perreault's algorithm"), complex_radio2_);

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
    lambda_spin_->setMaximum(1.0/4.0);
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
    klength_open_spin_->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));

    QFormLayout* open_layout = new QFormLayout;
    open_layout->addRow(tr("SE size ="), klength_open_spin_);

    close_groupbox_ = new QGroupBox(tr("Closing"));
    close_groupbox_->setCheckable(true);
    close_groupbox_->setChecked(false);
    klength_close_spin_ = new KernelSizeSpinBox;
    klength_close_spin_->setSingleStep(2);
    klength_close_spin_->setMinimum(3);
    klength_close_spin_->setMaximum(499);
    klength_close_spin_->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));

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
    klength_tophat_spin_->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));

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
    //connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_visu(int)) );

    connect(dial_buttons_, &QDialogButtonBox::accepted,
            this, &SettingsWindow::accept);

    connect(dial_buttons_, &QDialogButtonBox::rejected,
            this, &SettingsWindow::reject);

    auto* restoreBtn =
        dial_buttons_->button(QDialogButtonBox::RestoreDefaults);

    connect(restoreBtn, &QPushButton::clicked,
            this, &SettingsWindow::default_settings);

    //connect(klength_gradient_spin,SIGNAL(valueChanged(int)),this,
    //SLOT(show_phi_with_filtered_image()));

    //connect(alpha_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(beta_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(gamma_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    connect(width_slider_,      &QSlider::valueChanged,
            width_shape_spin_,  &QSpinBox::setValue);

    connect(width_shape_spin_,  QOverload<int>::of(&QSpinBox::valueChanged),
            width_slider_,      &QSlider::setValue);

    connect(height_slider_,      &QSlider::valueChanged,
            height_shape_spin_,  &QSpinBox::setValue);

    connect(height_shape_spin_,  QOverload<int>::of(&QSpinBox::valueChanged),
            height_slider_,      &QSlider::setValue);

    connect(ordinate_slider_,      &QSlider::valueChanged,
            ordinate_spin_,  &QSpinBox::setValue);

    connect(ordinate_spin_,  QOverload<int>::of(&QSpinBox::valueChanged),
            ordinate_slider_,      &QSlider::setValue);

    connect(abscissa_slider_,      &QSlider::valueChanged,
            abscissa_spin_,  &QSpinBox::setValue);

    connect(abscissa_spin_,  QOverload<int>::of(&QSpinBox::valueChanged),
            abscissa_slider_,      &QSlider::setValue);





    //connect(gaussian_noise_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(std_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

    //connect(salt_noise_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(proba_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

    //connect(speckle_noise_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(std_speckle_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

    //connect(mean_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(klength_mean_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    //connect(gaussian_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(klength_gaussian_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(std_filter_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

    //connect(median_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(klength_median_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(complex_radio1,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(complex_radio2,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

    //connect(aniso_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(aniso1_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(aniso2_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(iteration_filter_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(lambda_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));
    //connect(kappa_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

    //connect(open_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(klength_open_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    //connect(close_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(klength_close_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    //connect(tophat_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(whitetophat_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(blacktophat_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(klength_tophat_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    //connect(complex1_morpho_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(complex2_morpho_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

        //connect(preprocess_page,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

        //connect(histo_checkbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));


    connect(add_button_, &QPushButton::clicked,
            this, &SettingsWindow::onAddShape);

    connect(subtract_button_, &QPushButton::clicked,
            this, &SettingsWindow::onSubtractShape);

    connect(clear_button_, &QPushButton::clicked,
            this, &SettingsWindow::onClearPhi);

    connect(imageSettingsController_,
            &ImageSettingsController::viewChanged,
            settingsView_,
            &ImageView::setImage);

    //phiViewModel_->onConnectivityChanged( connectivity_cb->currentIndex() );

    /*connect(connectivity_cb,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            phiViewModel_,
            &PhiViewModel::onConnectivityChanged);*/

    connect(rectangle_radio_,   &QRadioButton::toggled,
            this,  &SettingsWindow::onUiShapeChanged);
    connect(ellipse_radio_,     &QRadioButton::toggled,
            this,  &SettingsWindow::onUiShapeChanged);

    connect(width_shape_spin_,  &QSpinBox::valueChanged,
            this,  &SettingsWindow::onUiShapeChanged);
    connect(height_shape_spin_, &QSpinBox::valueChanged,
            this,  &SettingsWindow::onUiShapeChanged);

    connect(abscissa_spin_,     &QSpinBox::valueChanged,
            this,  &SettingsWindow::onUiShapeChanged);
    connect(ordinate_spin_,     &QSpinBox::valueChanged,
            this,  &SettingsWindow::onUiShapeChanged);

    connect(this,
            &SettingsWindow::updateOverlay,
            imageSettingsController_,
            &ImageSettingsController::onUpdateOverlay);
}

void SettingsWindow::onUiShapeChanged()
{
    emit updateOverlay( getUiShape() );
}

UiShapeInfo SettingsWindow::getUiShape() const
{
    UiShapeInfo uiShape;

    if ( rectangle_radio_->isChecked() )
        uiShape.shape = ShapeType::Rectangle;
    else if ( ellipse_radio_->isChecked() )
        uiShape.shape = ShapeType::Ellipse;
    else
        uiShape.shape = ShapeType::Rectangle;

    uiShape.width = width_shape_spin_->value();
    uiShape.height = height_shape_spin_->value();
    uiShape.x = abscissa_spin_->value();
    uiShape.y = ordinate_spin_->value();

    return uiShape;
}

void SettingsWindow::accept()
{
    auto& ds_config = AppSettings::instance().imgConfig.compute.downscale;

    ds_config.hasDownscale = downscale_page_->isChecked();
    ds_config.downscaleFactor = downscale_factor_cb_->currentData().toInt();

    auto& filt_config = AppSettings::instance().imgConfig.compute.processing;

    filt_config.has_gaussian_noise = gaussian_noise_groupbox_->isChecked();
    filt_config.std_noise = float( std_noise_spin_->value() );
    filt_config.has_salt_noise = salt_noise_groupbox_->isChecked();
    filt_config.proba_noise = float( proba_noise_spin_->value() ) / 100.f;
    filt_config.has_speckle_noise = speckle_noise_groupbox_->isChecked();
    filt_config.std_speckle_noise = float( std_speckle_noise_spin_->value() );
    filt_config.has_median_filt = median_groupbox_->isChecked();
    filt_config.kernel_median_length = klength_median_spin_->value();
    filt_config.has_O1_algo = complex_radio2_->isChecked();

    filt_config.has_mean_filt = mean_groupbox_->isChecked();
    filt_config.kernel_mean_length = klength_mean_spin_->value();

    filt_config.has_gaussian_filt = gaussian_groupbox_->isChecked();
    filt_config.kernel_gaussian_length = klength_gaussian_spin_->value();
    filt_config.sigma = float( std_filter_spin_->value() );
    filt_config.has_aniso_diff = aniso_groupbox_->isChecked();

    filt_config.max_itera = iteration_filter_spin_->value();
    filt_config.lambda = float( lambda_spin_->value() );
    filt_config.kappa = float( kappa_spin_->value() );
    if( aniso1_radio_->isChecked() )
    {
        filt_config.aniso_option = ofeli_ip::AnisoDiff::FUNCTION1;
    }
    else if( aniso2_radio_->isChecked() )
    {
        filt_config.aniso_option = ofeli_ip::AnisoDiff::FUNCTION2;
    }

    filt_config.has_open_filt = open_groupbox_->isChecked();
    filt_config.kernel_open_length = klength_open_spin_->value();
    filt_config.has_close_filt = close_groupbox_->isChecked();
    filt_config.kernel_close_length = klength_close_spin_->value();
    filt_config.has_top_hat_filt = tophat_groupbox_->isChecked();
    filt_config.is_white_top_hat = whitetophat_radio_->isChecked();
    filt_config.kernel_tophat_length = klength_tophat_spin_->value();
    filt_config.has_O1_morpho = complex2_morpho_radio_->isChecked();

    imageSettingsController_->accept();

    algo_widget_->accept();

    // for data persistence
    AppSettings::instance().save_img_session_config();

    QDialog::accept();
}

void SettingsWindow::updateUIFromConfig()
{
    const auto& ds_config = AppSettings::instance().imgConfig.compute.downscale;

    downscale_page_->setChecked( ds_config.hasDownscale );

    int index = downscale_factor_cb_->findData( ds_config.downscaleFactor );
    if ( index >= 0 )
        downscale_factor_cb_->setCurrentIndex( index );

    const auto& config_filter = AppSettings::instance().imgConfig.compute.processing;

    gaussian_noise_groupbox_->setChecked(config_filter.has_gaussian_noise);
    std_noise_spin_->setValue( double( config_filter.std_noise ) );
    salt_noise_groupbox_->setChecked(config_filter.has_salt_noise);
    proba_noise_spin_->setValue( double( 100.f*config_filter.proba_noise ) );
    speckle_noise_groupbox_->setChecked(config_filter.has_speckle_noise);
    std_speckle_noise_spin_->setValue( double( config_filter.std_speckle_noise ) );

    median_groupbox_->setChecked(config_filter.has_median_filt);
    klength_median_spin_->setValue(config_filter.kernel_median_length);

    if( config_filter.has_O1_algo )
    {
        complex_radio2_->setChecked(true);
    }
    else
    {
        complex_radio1_->setChecked(true);
    }

    mean_groupbox_->setChecked(config_filter.has_mean_filt);
    klength_mean_spin_->setValue(config_filter.kernel_mean_length);

    gaussian_groupbox_->setChecked(config_filter.has_gaussian_filt);
    klength_gaussian_spin_->setValue(config_filter.kernel_gaussian_length);
    std_filter_spin_->setValue( double( config_filter.sigma ) );

    aniso_groupbox_->setChecked(config_filter.has_aniso_diff);
    iteration_filter_spin_->setValue(config_filter.max_itera);
    lambda_spin_->setValue( double( config_filter.lambda ) );
    kappa_spin_->setValue( double( config_filter.kappa ) );
    if( config_filter.aniso_option == ofeli_ip::AnisoDiff::FUNCTION1 )
    {
        aniso1_radio_->setChecked(true);
    }
    else if( config_filter.aniso_option == ofeli_ip::AnisoDiff::FUNCTION2 )
    {
        aniso2_radio_->setChecked(true);
    }

    open_groupbox_->setChecked(config_filter.has_open_filt);
    klength_open_spin_->setValue(config_filter.kernel_open_length);

    close_groupbox_->setChecked(config_filter.has_close_filt);
    klength_close_spin_->setValue(config_filter.kernel_close_length);

    tophat_groupbox_->setChecked(config_filter.has_top_hat_filt);

    if( config_filter.is_white_top_hat )
    {
        whitetophat_radio_->setChecked(true);
    }
    else
    {
        blacktophat_radio_->setChecked(true);
    }
    klength_tophat_spin_->setValue(config_filter.kernel_tophat_length);

    if( config_filter.has_O1_morpho )
    {
        complex2_morpho_radio_->setChecked(true);
    }
    else
    {
        complex1_morpho_radio_->setChecked(true);
    }

    imageSettingsController_->reject();

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
    connectivity_cb->setCurrentIndex( int(ofeli_ip::Connectivity::Four) );

    Na_spin->setValue( 30 );
    lambda_in_spin->setValue( 1 );
    lambda_out_spin->setValue( 1 );

    color_space_cb->setCurrentIndex( int(ofeli_ip::ColorSpaceOption::RGB) );

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

    preprocess_page_->setChecked( false );

    gaussian_noise_groupbox_->setChecked( false );
    std_noise_spin_->setValue( 20.0 );

    salt_noise_groupbox_->setChecked( false );
    proba_noise_spin_->setValue( 0.05 );

    speckle_noise_groupbox_->setChecked(false);
    std_speckle_noise_spin_->setValue( 0.16 );

    median_groupbox_->setChecked( false );
    klength_median_spin_->setValue( 5 );

    // has_O1_algo
    complex_radio2_->setChecked( true );

    mean_groupbox_->setChecked( false );
    klength_mean_spin_->setValue( 5 );

    gaussian_groupbox_->setChecked( false );
    klength_gaussian_spin_->setValue( 5 );
    std_filter_spin_->setValue( 2.0 );

    aniso_groupbox_->setChecked( false );
    // AnisoDiff::FUNCTION1
    aniso1_radio_->setChecked( true );
    iteration_filter_spin_->setValue( 10 );
    lambda_spin_->setValue( 1.0/7.0 );
    kappa_spin_->setValue( 30.0 );

    open_groupbox_->setChecked( false );
    klength_open_spin_->setValue( 5 );

    close_groupbox_->setChecked( false );
    klength_close_spin_->setValue( 5 );

    tophat_groupbox_->setChecked( false );
    whitetophat_radio_->setChecked( true );
    klength_tophat_spin_->setValue( 5 );

    // config.has_O1_morpho
    complex2_morpho_radio_->setChecked( true );

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    ellipse_radio_->setChecked( true );

    // 65% width and 65% height by default
    width_shape_spin_->setValue(65);
    height_shape_spin_->setValue(65);

    // centered by default
    abscissa_spin_->setValue(0);
    ordinate_spin_->setValue(0);
}

/*
const unsigned char* SettingsWindow::get_filtered_img_data()
{
    const auto& config = AppSettings::instance();

    if(    config.has_preprocess
        && ( config.has_gaussian_noise || config.has_salt_noise || config.has_speckle_noise ) )
    {
        calculate_filtered_image();
    }

    return img2_filtered;
}

float SettingsWindow::calculate_filtered_image()
{
    // pour refiltrer à partir de l'image de départ
    filters2->initialyze_filtered();

    float elapsed_time;
    std::clock_t start_time, stop_time;

    start_time = std::clock();

    if( has_preprocess2 )
    {
        if( has_gaussian_noise2 )
        {
            filters2->gaussian_white_noise(std_noise2);
        }
        if( has_salt_noise2 )
        {
            filters2->salt_pepper_noise(proba_noise2);
        }
        if( has_speckle_noise2 )
        {
            filters2->speckle(std_speckle_noise2);
        }

        if( has_mean_filt2 )
        {
            filters2->mean_filtering(kernel_mean_length2);
        }
        if( has_gaussian_filt2 )
        {
            filters2->gaussian_filtering(kernel_gaussian_length2, sigma2);
        }

        if( has_median_filt2 )
        {
            if( has_O1_algo2 )
            {
                filters2->median_filtering_o1(kernel_median_length2);
            }
            else
            {
                filters2->median_filtering_oNlogN(kernel_median_length2);
            }
        }
        if( has_aniso_diff2 )
        {
            filters2->anisotropic_diffusion(max_itera2, lambda2, kappa2, aniso_option2);
        }

        if( has_open_filt2 )
        {
            if( has_O1_morpho2 )
            {
                filters2->opening_o1(kernel_open_length2);
            }
            else
            {
                filters2->opening(kernel_open_length2);
            }
        }

        if( has_close_filt2 )
        {
            if( has_O1_morpho2 )
            {
                filters2->closing_o1(kernel_close_length2);
            }
            else
            {
                filters2->closing(kernel_close_length2);
            }
        }

        if( has_top_hat_filt2 )
        {
            if( is_white_top_hat2 )
            {
                if( has_O1_morpho2 )
                {
                    filters2->white_top_hat_o1(kernel_tophat_length2);
                }
                else
                {
                    filters2->white_top_hat(kernel_tophat_length2);
                }
            }
            else
            {
                if( has_O1_morpho2 )
                {
                    filters2->black_top_hat_o1(kernel_tophat_length2);
                }
                else
                {
                    filters2->black_top_hat(kernel_tophat_length2);
                }
            }
        }
    }

    if( speed == SpeedModel::REGION_BASED )
    {
        img2_filtered = filters2->get_filtered();
    }
    else if( speed == SpeedModel::EDGE_BASED )
    {
        if( is_rgb1 )
        {
            filters2->morphological_gradient_yuv(kernel_gradient_length2,alpha2,beta2,gamma2);
            img2_filtered = filters2->get_gradient();
        }
        else
        {
            filters2->morphological_gradient(kernel_gradient_length2);
            img2_filtered = filters2->get_filtered();
        }
    }

    stop_time = std::clock();
    elapsed_time = float(stop_time - start_time) / float(CLOCKS_PER_SEC);

    return elapsed_time;
}*/

void SettingsWindow::onAddShape()
{
    if ( imageSettingsController_ )
        imageSettingsController_->addShape( getUiShape() );
}

void SettingsWindow::onSubtractShape()
{
    if ( imageSettingsController_ )
        imageSettingsController_->subtractShape( getUiShape() );
}

void SettingsWindow::onClearPhi()
{
    if ( imageSettingsController_ )
        imageSettingsController_->clearPhi();
}

void SettingsWindow::onInputImageReady(const QImage& inputImage)
{
    if ( !inputImage.isNull() )
        imageSettingsController_->onInputImageReady(inputImage);
}

void SettingsWindow::showEvent(QShowEvent* /*event*/)
{
    emit updateOverlay( getUiShape() );
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue( "Settings/Window/geometry", saveGeometry() );

    QDialog::closeEvent(event);
}

}
