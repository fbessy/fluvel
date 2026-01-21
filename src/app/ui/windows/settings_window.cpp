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
#include "contour_rendering.hpp"
#include "application_settings.hpp"

#include "image_window.hpp"
#include "filters.hpp"
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"
#include "kernel_size_spinbox.hpp"
#include "boundary_builder.hpp"
#include "active_contour.hpp"
#include "shape_overlay_renderer.hpp"
#include "phi_view_model.hpp"
#include "shape_type.hpp"

#include <QtWidgets>

#include <stack>
#include <ctime>         // for std::clock_t, std::clock() and CLOCKS_PER_SEC
#include <cstring>       // for std::memcpy

namespace ofeli_app
{

SettingsWindow::SettingsWindow(QWidget* parent,
                               PhiEditor* phiEditor,
                               PhiViewModel* phiViewModel) :
    QDialog(parent),
    phiEditor_(phiEditor),
    phiViewModel_(phiViewModel),
    filters2(nullptr),
    img2_filtered(nullptr),
    img1(nullptr)
{
    setWindowTitle(tr("Settings"));

    QSettings settings;

    const auto geo = settings.value("Settings/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    dial_buttons = new QDialogButtonBox(this);
    dial_buttons->addButton(QDialogButtonBox::Ok);
    dial_buttons->addButton(QDialogButtonBox::Cancel);
    dial_buttons->addButton(QDialogButtonBox::Reset);




    setupUiAlgoTab();
    setupUiInitTab();
    setupUiPreprocessingTab();
    setupUiDisplayTab();

    tabs = new QTabWidget(this);
    tabs->addTab( page1, tr("Algorithm") );
    tabs->addTab( page2, tr("Initialization") );
    tabs->addTab( page3, tr("Preprocessing") );
    tabs->addTab( page4, tr("Display") );


    settingsView = new ImageView(this);

    QGridLayout *settings_grid = new QGridLayout;
    settings_grid->addWidget(tabs,0,0);
    settings_grid->addWidget(settingsView,0,1);

    settings_grid->addWidget(dial_buttons,1,1);

    settings_grid->setColumnStretch(1,1);
    setLayout(settings_grid);

    last_directory_used = settings.value("Main/Name/last_directory_used",QDir().homePath()).toString();

    has_contours_hidden = true;

    reject();

    setupConnections();
}

void SettingsWindow::setupUiAlgoTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Algorithm tab
    ////////////////////////////////////////////////////////////////////////////////////////////////


    connectivity_cb = new QComboBox;
    connectivity_cb->addItem("4-connected (Von Neumann)");
    connectivity_cb->addItem("8-connected (Moore)");

    QFormLayout* connect_layout = new QFormLayout;
    connect_layout->addRow("Connectivity :", connectivity_cb);

    QGroupBox* externalspeed_groupbox = new QGroupBox(tr("Cycle 1 – Data-driven evolution"));

    Na_spin = new QSpinBox;
    Na_spin->setSingleStep(1);
    Na_spin->setMinimum(1);
    Na_spin->setMaximum(999);
    Na_spin->setSuffix(tr(" iterations"));
    Na_spin->setToolTip(tr("Number of iterations of the data-driven evolution (Cycle 1)."));
    QFormLayout *Na_layout = new QFormLayout;
    Na_layout->addRow("Na =", Na_spin);

    klength_gradient_spin = new KernelSizeSpinBox;
    klength_gradient_spin->setSingleStep(2);
    klength_gradient_spin->setMinimum(3);
    klength_gradient_spin->setMaximum(499);

    chanvese_radio = new QRadioButton(tr("Chan-Vese model"));
    chanvese_radio->setToolTip(tr("region-based model for bimodal images"));

    lambda_out_spin = new QSpinBox;
    lambda_out_spin->setSingleStep(1);
    lambda_out_spin->setMinimum(1);
    lambda_out_spin->setMaximum(100000);
    lambda_out_spin->setToolTip(tr("weight of the outside homogeneity criterion"));

    lambda_in_spin = new QSpinBox;
    lambda_in_spin->setSingleStep(1);
    lambda_in_spin->setMinimum(1);
    lambda_in_spin->setMaximum(100000);
    lambda_in_spin->setToolTip(tr("weight of the inside homogeneity criterion"));

    QFormLayout* lambda_layout = new QFormLayout;

    auto& config = AppSettings::instance();

    QColor RGBout_list( get_QRgb(config.color_out) );
    QColor RGBin_list( get_QRgb(config.color_in) );

    lambda_layout->addRow("<font color="+RGBout_list.name()+">"+"λout"+"<font color=black>"+" =", lambda_out_spin);
    lambda_layout->addRow("<font color="+RGBin_list.name()+">"+"λin"+"<font color=black>"+" =", lambda_in_spin);

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addWidget(chanvese_radio);
    chanvese_layout->addLayout(lambda_layout);

    geodesic_radio = new QRadioButton(tr("geodesic model"));
    geodesic_radio->setToolTip(tr("edge-based model for smoothed multimodal images"));

    klength_gradient_spin = new KernelSizeSpinBox;
    klength_gradient_spin->setSingleStep(2);
    klength_gradient_spin->setMinimum(3);
    klength_gradient_spin->setMaximum(499);
    klength_gradient_spin->setToolTip(tr("morphological gradient structuring element size"));
    QFormLayout *gradient_layout = new QFormLayout;
    gradient_layout->addRow(tr("SE size ="), klength_gradient_spin);

    QVBoxLayout* geodesic_layout = new QVBoxLayout;
    geodesic_layout->addWidget(geodesic_radio);
    geodesic_layout->addLayout(gradient_layout);

    QHBoxLayout* speed_layout = new QHBoxLayout;
    speed_layout->addLayout(chanvese_layout);
    speed_layout->addLayout(geodesic_layout);

    color_weights_groupbox = new QGroupBox(tr("Color space"));
    color_weights_groupbox->setFlat(true);

    color_space_cb = new QComboBox;
    color_space_cb->addItem("RGB");
    color_space_cb->addItem("YUV");
    color_space_cb->addItem("L*a*b* (CIELAB)");
    color_space_cb->addItem("L*u*v* (CIELUV)");

    alpha_spin = new QSpinBox;
    alpha_spin->setSingleStep(1);
    alpha_spin->setMinimum(1);
    alpha_spin->setMaximum(100000);
    alpha_spin->setToolTip(tr("perceptual lightness weight"));

    beta_spin = new QSpinBox;
    beta_spin->setSingleStep(1);
    beta_spin->setMinimum(1);
    beta_spin->setMaximum(100000);
    beta_spin->setToolTip(tr("green-red chrominance weight"));

    gamma_spin = new QSpinBox;
    gamma_spin->setSingleStep(1);
    gamma_spin->setMinimum(1);
    gamma_spin->setMaximum(100000);
    gamma_spin->setToolTip(tr("blue–yellow chrominance weight"));



    QFormLayout* color_weights_layout = new QFormLayout;
    color_weights_layout->addRow(tr("1st component weight ="), alpha_spin);
    color_weights_layout->addRow(tr("2nd component weight ="), beta_spin);
    color_weights_layout->addRow(tr("3rd component weight ="), gamma_spin);

    QVBoxLayout* vcolor_layout = new QVBoxLayout;
    vcolor_layout->addWidget(color_space_cb);
    vcolor_layout->addLayout(color_weights_layout);

    color_weights_groupbox->setLayout(vcolor_layout);



    QVBoxLayout* externalspeed_layout = new QVBoxLayout;

    externalspeed_layout->addLayout(Na_layout);
    externalspeed_layout->addLayout(speed_layout);
    externalspeed_layout->addWidget(color_weights_groupbox);
    externalspeed_groupbox->setLayout(externalspeed_layout);

    ////////////////////////////////////////////

    internalspeed_groupbox = new QGroupBox(tr("Cycle 2 - Internal smoothing"));
    internalspeed_groupbox->setCheckable(true);
    internalspeed_groupbox->setChecked(true);

    Ns_spin = new QSpinBox;
    Ns_spin->setSingleStep(1);
    Ns_spin->setMinimum(1);
    Ns_spin->setMaximum(999);
    Ns_spin->setSuffix(tr(" iterations"));
    Ns_spin->setToolTip(tr("Number of internal smoothing iterations (Cycle 2)."));

    disk_radius_spin = new QSpinBox;
    disk_radius_spin->setSingleStep(1);
    disk_radius_spin->setMinimum(1);
    disk_radius_spin->setMaximum(999);
    disk_radius_spin->setToolTip(tr("Radius of the disk-shaped neighborhood used for the majority vote during internal smoothing."));

    QFormLayout* internalspeed_layout = new QFormLayout;
    internalspeed_layout->addRow("Ns =", Ns_spin);
    internalspeed_layout->addRow("R =", disk_radius_spin);

    internalspeed_groupbox->setLayout(internalspeed_layout);

    ////////////////////////////////////////////

    QGroupBox* tracking_groupbox = new QGroupBox(tr("Video tracking"));

    downscale_factor_cb = new QComboBox;
    downscale_factor_cb->addItem("1");
    downscale_factor_cb->addItem("2");
    downscale_factor_cb->addItem("4");


    cycles_nbr_sb = new QSpinBox;
    cycles_nbr_sb->setMinimum(1);

    QFormLayout* cycles_nbr_layout = new QFormLayout;
    cycles_nbr_layout->addRow("downscale factor = ", downscale_factor_cb);
    cycles_nbr_layout->addRow("cycles per frame = ", cycles_nbr_sb);

    tracking_groupbox->setLayout(cycles_nbr_layout);



    QVBoxLayout* algorithm_layout = new QVBoxLayout;
    algorithm_layout->addLayout(connect_layout);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(externalspeed_groupbox);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(internalspeed_groupbox);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(tracking_groupbox);
    algorithm_layout->addStretch(1);

    page1 = new QWidget;
    page1->setLayout(algorithm_layout);
}

void SettingsWindow::setupUiInitTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialization tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    open_phi_button = new QPushButton(tr("Open ϕ(t=0)"));
    open_phi_button->setToolTip(tr("or drag and drop an image of ϕ(t=0)"));
    save_phi_button = new QPushButton(tr("Save ϕ(t=0)"));

    open_phi_button->setEnabled(false);
    save_phi_button->setEnabled(false);


    QHBoxLayout* openSavePhi= new QHBoxLayout;
    openSavePhi->addWidget(open_phi_button);
    openSavePhi->addWidget(save_phi_button);

    ////////////////////////////////////////////////////

    QGroupBox* shape_groupbox = new QGroupBox(tr("Shape"));
    shape_groupbox->setFlat(true);

    rectangle_radio = new QRadioButton(tr("rectangle"));
    rectangle_radio->setToolTip(tr("or click on the middle mouse button when the cursor is in the image"));
    ellipse_radio = new QRadioButton(tr("ellipse"));
    ellipse_radio->setToolTip(tr("or click on the middle mouse button when the cursor is in the image"));

    rectangle_radio->setChecked(false);
    ellipse_radio->setChecked(true);

    QHBoxLayout* shape_layout = new QHBoxLayout;
    shape_layout->addWidget(rectangle_radio);
    shape_layout->addWidget(ellipse_radio);
    shape_groupbox->setLayout(shape_layout);

    ////////////////////////////////////////////

    QGroupBox* shape_size_groupbox = new QGroupBox(tr("Size"));
    shape_size_groupbox ->setToolTip(tr("or roll the mouse wheel when the cursor is in the image"));

    width_shape_spin = new QSpinBox;
    width_shape_spin->setSingleStep(15);
    width_shape_spin->setMinimum(0);
    width_shape_spin->setMaximum(200);
    width_shape_spin->setSuffix(tr(" % image width"));
    width_shape_spin->setValue(65);

    QFormLayout* width_spin_layout = new QFormLayout;
    width_spin_layout->addRow(tr("width ="), width_shape_spin);

    width_slider = new QSlider(Qt::Horizontal, this);

    width_slider->setTickPosition(QSlider::TicksAbove);

    width_slider->setMinimum(0);
    width_slider->setMaximum(200);
    width_slider->setTickInterval(25);
    width_slider->setSingleStep(15);
    width_slider->setValue(65);



    height_shape_spin = new QSpinBox;
    height_shape_spin->setSingleStep(15);
    height_shape_spin->setMinimum(0);
    height_shape_spin->setMaximum(200);
    height_shape_spin->setSuffix(tr((" % image height")));
    height_shape_spin->setValue(65);

    QFormLayout* height_spin_layout = new QFormLayout;
    height_spin_layout->addRow(tr("height ="), height_shape_spin);

    height_slider = new QSlider(Qt::Horizontal, this);

    height_slider->setTickPosition(QSlider::TicksAbove);

    height_slider->setMinimum(0);
    height_slider->setMaximum(200);
    height_slider->setTickInterval(25);
    height_slider->setSingleStep(15);
    height_slider->setValue(65);



    QVBoxLayout* shape_size_layout = new QVBoxLayout;
    shape_size_layout->addLayout(width_spin_layout);
    shape_size_layout->addWidget(width_slider);
    shape_size_layout->addLayout(height_spin_layout);
    shape_size_layout->addWidget(height_slider);
    shape_size_groupbox->setLayout(shape_size_layout);

    ////////////////////////////////////////////

    QGroupBox* position_groupbox = new QGroupBox(tr("Position (x,y)"));
    position_groupbox->setToolTip(tr("or move the mouse cursor in the image"));

    abscissa_spin = new QSpinBox;
    abscissa_spin->setSingleStep(15);
    abscissa_spin->setMinimum(-200);
    abscissa_spin->setMaximum(200);
    abscissa_spin->setSuffix(tr(" % image width"));
    abscissa_spin->setValue(0);

    QFormLayout* abscissa_spin_layout = new QFormLayout;
    abscissa_spin_layout->addRow("x = Xo +", abscissa_spin);

    abscissa_slider = new QSlider(Qt::Horizontal, this);

    abscissa_slider->setTickPosition(QSlider::TicksAbove);

    abscissa_slider->setMinimum(-75);
    abscissa_slider->setMaximum(75);
    abscissa_slider->setTickInterval(25);
    abscissa_slider->setSingleStep(15);
    abscissa_slider->setValue(0);



    ordinate_spin = new QSpinBox;
    ordinate_spin->setSingleStep(15);
    ordinate_spin->setMinimum(-200);
    ordinate_spin->setMaximum(200);
    ordinate_spin->setSuffix(tr(" % image height"));
    ordinate_spin->setValue(0);

    QFormLayout* ordinate_spin_layout = new QFormLayout;
    ordinate_spin_layout->addRow("y = Yo +", ordinate_spin);

    ordinate_slider = new QSlider(Qt::Horizontal, this);

    ordinate_slider->setTickPosition(QSlider::TicksAbove);

    ordinate_slider->setMinimum(-75);
    ordinate_slider->setMaximum(75);
    ordinate_slider->setTickInterval(25);
    ordinate_slider->setSingleStep(15);
    ordinate_slider->setValue(0);



    QVBoxLayout* position_layout = new QVBoxLayout;
    position_layout->addLayout(abscissa_spin_layout);
    position_layout->addWidget(abscissa_slider);
    position_layout->addLayout(ordinate_spin_layout);
    position_layout->addWidget(ordinate_slider);
    position_groupbox->setLayout(position_layout);

    ////////////////////////////////////////////

    QGroupBox* modify_groupbox = new QGroupBox(tr("Active contour modification"));
    modify_groupbox->setFlat(true);

    add_button = new QPushButton(tr("Add"));
    add_button->setToolTip(tr("or click on the left mouse button when the cursor is in the image"));

    subtract_button = new QPushButton(tr("Subtract"));
    subtract_button->setToolTip(tr("or click on the right mouse button when the cursor is in the image"));

    clear_button = new QPushButton(tr("Clear"));

    QHBoxLayout* modify_layout = new QHBoxLayout;
    modify_layout->addWidget(clear_button);
    modify_layout->addWidget(subtract_button);
    modify_layout->addWidget(add_button);
    modify_groupbox->setLayout(modify_layout);

    ////////////////////////////////////////////

    QVBoxLayout* initialization_layout = new QVBoxLayout;
    initialization_layout->addLayout(openSavePhi);
    initialization_layout->addWidget(shape_groupbox);
    initialization_layout->addWidget(shape_size_groupbox);
    initialization_layout->addWidget(position_groupbox);
    initialization_layout->addWidget(modify_groupbox);
    initialization_layout->addStretch(1);

    page2 = new QWidget;
    page2->setLayout(initialization_layout);
}

void SettingsWindow::setupUiPreprocessingTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Preprocessing tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    gaussian_noise_groupbox = new QGroupBox(tr("Gaussian white noise"));
    gaussian_noise_groupbox->setCheckable(true);
    gaussian_noise_groupbox->setChecked(false);
    std_noise_spin = new QDoubleSpinBox;
    std_noise_spin->setSingleStep(5.0);
    std_noise_spin->setMinimum(0.0);
    std_noise_spin->setMaximum(10000.0);
    std_noise_spin->setToolTip(tr("standard deviation"));


    QFormLayout* gaussian_noise_layout = new QFormLayout;
    gaussian_noise_layout->addRow("σ =", std_noise_spin);

    salt_noise_groupbox = new QGroupBox(tr("Impulsional noise (salt and pepper)"));
    salt_noise_groupbox->setCheckable(true);
    salt_noise_groupbox->setChecked(false);
    proba_noise_spin = new QDoubleSpinBox;
    proba_noise_spin->setSingleStep(1.0);
    proba_noise_spin->setMinimum(0.0);
    proba_noise_spin->setMaximum(100.0);
    proba_noise_spin->setSuffix(" %");
    proba_noise_spin->setToolTip(tr("impulsional noise probability for each pixel"));


    QFormLayout* salt_noise_layout = new QFormLayout;
    salt_noise_layout->addRow(tr("d ="), proba_noise_spin);

    speckle_noise_groupbox = new QGroupBox(tr("Speckle noise"));
    speckle_noise_groupbox->setCheckable(true);
    speckle_noise_groupbox->setChecked(false);
    std_speckle_noise_spin = new QDoubleSpinBox;
    std_speckle_noise_spin->setSingleStep(0.02);
    std_speckle_noise_spin->setMinimum(0.0);
    std_speckle_noise_spin->setMaximum(1000.0);
    std_speckle_noise_spin->setToolTip(tr("standard deviation"));


    QFormLayout* speckle_noise_layout = new QFormLayout;
    speckle_noise_layout->addRow("σ =", std_speckle_noise_spin);

    gaussian_noise_groupbox->setLayout(gaussian_noise_layout);
    salt_noise_groupbox->setLayout(salt_noise_layout);
    speckle_noise_groupbox->setLayout(speckle_noise_layout);

    QVBoxLayout* noise_layout = new QVBoxLayout;
    noise_layout->addWidget(gaussian_noise_groupbox);
    noise_layout->addWidget(salt_noise_groupbox);
    noise_layout->addWidget(speckle_noise_groupbox);
    noise_layout->addStretch(1);

    ////////////////////////////////////////////

    mean_groupbox = new QGroupBox(tr("Mean filter"));
    mean_groupbox->setCheckable(true);
    mean_groupbox->setChecked(false);
    klength_mean_spin = new KernelSizeSpinBox;
    klength_mean_spin->setSingleStep(2);
    klength_mean_spin->setMinimum(3);
    klength_mean_spin->setMaximum(499);


    QFormLayout* mean_layout = new QFormLayout;
    mean_layout->addRow(tr("kernel size ="), klength_mean_spin);

    gaussian_groupbox = new QGroupBox(tr("Gaussian filter"));
    gaussian_groupbox->setCheckable(true);
    gaussian_groupbox->setChecked(false);
    klength_gaussian_spin = new KernelSizeSpinBox;
    klength_gaussian_spin->setSingleStep(2);
    klength_gaussian_spin->setMinimum(3);
    klength_gaussian_spin->setMaximum(499);
    std_filter_spin = new QDoubleSpinBox;
    std_filter_spin->setSingleStep(0.1);
    std_filter_spin->setMinimum(0.0);
    std_filter_spin->setMaximum(1000000.0);
    std_filter_spin->setToolTip(tr("standard deviation"));

    QFormLayout* gaussian_layout = new QFormLayout;
    gaussian_layout->addRow(tr("kernel size ="), klength_gaussian_spin);
    gaussian_layout->addRow("σ =", std_filter_spin);

    ////////////////////////////////////////////

    median_groupbox = new QGroupBox(tr("Median filter"));
    median_groupbox->setCheckable(true);
    median_groupbox->setChecked(false);
    klength_median_spin = new KernelSizeSpinBox;
    klength_median_spin->setSingleStep(2);
    klength_median_spin->setMinimum(3);
    klength_median_spin->setMaximum(499);
    //klength_median_spin->setSuffix("²");
    complex_radio1= new QRadioButton("O(r log r)×O(n)");
    complex_radio2= new QRadioButton("O(1)×O(n)");

    QFormLayout* median_layout = new QFormLayout;
    median_layout->addRow(tr("kernel size ="), klength_median_spin);
    median_layout->addRow(tr("quick sort algorithm"), complex_radio1);
    median_layout->addRow(tr("Perreault's algorithm"), complex_radio2);

    aniso_groupbox = new QGroupBox(tr("Perona-Malik anisotropic diffusion"));
    aniso_groupbox->setCheckable(true);
    aniso_groupbox->setChecked(false);
    aniso1_radio = new QRadioButton("g(∇I) = exp(-(|∇I|/κ)²)");
    aniso2_radio = new QRadioButton("g(∇I) = 1/(1+(1+(|∇I|/κ)²)");
    iteration_filter_spin = new QSpinBox;
    iteration_filter_spin->setSingleStep(1);
    iteration_filter_spin->setMinimum(0);
    iteration_filter_spin->setMaximum(5000);
    lambda_spin = new QDoubleSpinBox;
    lambda_spin->setSingleStep(0.01);
    lambda_spin->setMinimum(0.0);
    lambda_spin->setMaximum(1.0/4.0);
    kappa_spin = new QDoubleSpinBox;
    kappa_spin->setSingleStep(1.0);
    kappa_spin->setMinimum(0.0);
    kappa_spin->setMaximum(10000.0);

    QFormLayout* aniso_layout = new QFormLayout;
    aniso_layout->addRow(tr("iterations ="), iteration_filter_spin);
    aniso_layout->addRow("λ =", lambda_spin);
    aniso_layout->addRow("κ =", kappa_spin);
    aniso_layout->addRow(tr("function 1 :"), aniso1_radio);
    aniso_layout->addRow(tr("function 2 :"), aniso2_radio);

    ////////////////////////////////////////////

    open_groupbox = new QGroupBox(tr("Opening"));
    open_groupbox->setCheckable(true);
    open_groupbox->setChecked(false);
    klength_open_spin = new KernelSizeSpinBox;
    klength_open_spin->setSingleStep(2);
    klength_open_spin->setMinimum(3);
    klength_open_spin->setMaximum(499);
    klength_open_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));

    QFormLayout* open_layout = new QFormLayout;
    open_layout->addRow(tr("SE size ="), klength_open_spin);

    close_groupbox = new QGroupBox(tr("Closing"));
    close_groupbox->setCheckable(true);
    close_groupbox->setChecked(false);
    klength_close_spin = new KernelSizeSpinBox;
    klength_close_spin->setSingleStep(2);
    klength_close_spin->setMinimum(3);
    klength_close_spin->setMaximum(499);
    klength_close_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));

    QFormLayout* close_layout = new QFormLayout;
    close_layout->addRow(tr("SE size ="), klength_close_spin);

    ////////////////////////////////////////////

    tophat_groupbox = new QGroupBox(tr("Top-hat transform"));
    tophat_groupbox->setCheckable(true);
    tophat_groupbox->setChecked(false);
    whitetophat_radio = new QRadioButton(tr("white top-hat"));
    whitetophat_radio->setToolTip(tr("difference between the input image the opened"));
    blacktophat_radio = new QRadioButton(tr("black top-hat"));
    blacktophat_radio->setToolTip(tr("difference between the closed and the input image"));
    klength_tophat_spin = new KernelSizeSpinBox;
    klength_tophat_spin->setSingleStep(2);
    klength_tophat_spin->setMinimum(3);
    klength_tophat_spin->setMaximum(499);
    klength_tophat_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));

    QFormLayout* tophat_layout = new QFormLayout;
    tophat_layout->addRow(" ", whitetophat_radio);
    tophat_layout->addRow(" ", blacktophat_radio);
    tophat_layout->addRow(tr("SE size ="), klength_tophat_spin);

    algo_groupbox = new QGroupBox(tr("Algorithm"));
    complex1_morpho_radio = new QRadioButton(tr("naïve algorithm in O(r)×O(n)"));
    complex2_morpho_radio = new QRadioButton(tr("Perreault's algorithm in O(1)×O(n)"));
    QVBoxLayout* algo_layout = new QVBoxLayout;
    algo_layout->addWidget(complex1_morpho_radio);
    algo_layout->addWidget(complex2_morpho_radio);

    mean_groupbox->setLayout(mean_layout);
    gaussian_groupbox->setLayout(gaussian_layout);

    median_groupbox->setLayout(median_layout);
    aniso_groupbox->setLayout(aniso_layout);

    open_groupbox->setLayout(open_layout);
    close_groupbox->setLayout(close_layout);
    tophat_groupbox->setLayout(tophat_layout);
    algo_groupbox->setLayout(algo_layout);

    QVBoxLayout* filter_layout_linear = new QVBoxLayout;
    filter_layout_linear->addWidget(mean_groupbox);
    filter_layout_linear->addWidget(gaussian_groupbox);

    QVBoxLayout* filter_layout_edge_preserv = new QVBoxLayout;
    filter_layout_edge_preserv->addWidget(median_groupbox);
    filter_layout_edge_preserv->addWidget(aniso_groupbox);

    QVBoxLayout* filter_layout_mm = new QVBoxLayout;
    filter_layout_mm->addWidget(open_groupbox);
    filter_layout_mm->addWidget(close_groupbox);
    filter_layout_mm->addWidget(tophat_groupbox);
    filter_layout_mm->addWidget(algo_groupbox);

    ////////////////////////////////////////////

    preprocess_tabs = new QTabWidget(this);

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

    preprocess_tabs->addTab(page_noise, tr("Noise generators"));
    preprocess_tabs->addTab(filter_tabs, tr("Filters"));

    page3 = new QGroupBox(tr("Preprocessing"));
    page3->setCheckable(true);
    page3->setChecked(false);


    time_filt = new QLabel(this);
    time_filt->setText(tr("time = "));
    QGroupBox* time_filt_groupbox = new QGroupBox(tr("Processing time"));
    QVBoxLayout* elapsed_filt_layout = new QVBoxLayout;
    elapsed_filt_layout->addWidget(time_filt);
    time_filt_groupbox->setLayout(elapsed_filt_layout);

    QVBoxLayout* page3_layout = new QVBoxLayout;
    page3_layout->addWidget(preprocess_tabs);
    page3_layout->addWidget(time_filt_groupbox);
    page3->setLayout(page3_layout);
}

void SettingsWindow::setupUiDisplayTab()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Display tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* size_groupbox = new QGroupBox(tr("Image"));

    histo_checkbox = new QCheckBox(tr("histogram normalization for the gradient"));


    QFormLayout* size_layout = new QFormLayout;
    size_layout->addRow(" ",histo_checkbox);

    size_groupbox->setLayout(size_layout);

    ////////////////////////////////////////////

    step_checkbox = new QCheckBox(tr("display after each iteration"));
    step_checkbox->setToolTip(tr("It doesn't concern the video tracking part."));

    QGroupBox* active_contour_groupbox = new QGroupBox(tr("Active contour"));

    outsidecolor_combobox = new QComboBox;
    insidecolor_combobox = new QComboBox;

    // QPixmap pm : petite image affichant la couleur devant le nom de la couleur dans le combobox
    QPixmap pm(12,12);
    pm.fill(Qt::red);
    outsidecolor_combobox->addItem (pm, tr("Red"));
    insidecolor_combobox->addItem (pm, tr("Red"));
    pm.fill(Qt::green);
    outsidecolor_combobox->addItem (pm, tr("Green"));
    insidecolor_combobox->addItem (pm, tr("Green"));
    pm.fill(Qt::blue);
    outsidecolor_combobox->addItem (pm, tr("Blue"));
    insidecolor_combobox->addItem (pm, tr("Blue"));
    pm.fill(Qt::cyan);
    outsidecolor_combobox->addItem (pm, tr("Cyan"));
    insidecolor_combobox->addItem (pm, tr("Cyan"));
    pm.fill(Qt::magenta);
    outsidecolor_combobox->addItem (pm, tr("Magenta"));
    insidecolor_combobox->addItem (pm, tr("Magenta"));
    pm.fill(Qt::yellow);
    outsidecolor_combobox->addItem (pm, tr("Yellow"));
    insidecolor_combobox->addItem (pm, tr("Yellow"));
    pm.fill(Qt::black);
    outsidecolor_combobox->addItem (pm, tr("Black"));
    insidecolor_combobox->addItem (pm, tr("Black"));
    pm.fill(Qt::white);
    outsidecolor_combobox->addItem (pm, tr("White"));
    insidecolor_combobox->addItem (pm, tr("White"));
    pm.fill(Qt::transparent);
    outsidecolor_combobox->addItem (pm,tr("Selected"));
    insidecolor_combobox->addItem (pm,tr("Selected"));
    outsidecolor_combobox->addItem (pm, tr("No"));
    insidecolor_combobox->addItem (pm, tr("No"));

    outsidecolor_select = new QPushButton(tr("Select"));
    insidecolor_select = new QPushButton(tr("Select"));

    QFormLayout* Loutcolor_form = new QFormLayout;
    Loutcolor_form->addRow(tr("Lout :"), outsidecolor_combobox);

    QFormLayout* Lincolor_form = new QFormLayout;
    Lincolor_form->addRow(tr("   Lin :"), insidecolor_combobox);

    QHBoxLayout* Loutcolor_hlay = new QHBoxLayout;
    Loutcolor_hlay->addLayout(Loutcolor_form);
    Loutcolor_hlay->addWidget(outsidecolor_select);

    QHBoxLayout* Lincolor_hlay = new QHBoxLayout;
    Lincolor_hlay->addLayout(Lincolor_form);
    Lincolor_hlay->addWidget(insidecolor_select);

    QVBoxLayout* color_layout = new QVBoxLayout;
    color_layout->addLayout(Loutcolor_hlay);
    color_layout->addLayout(Lincolor_hlay);

    QGroupBox* color_groupbox = new QGroupBox(tr("boundaries lists colors"));
    color_groupbox->setFlat(true);
    color_groupbox->setLayout(color_layout);

    QVBoxLayout* active_contour_layout = new QVBoxLayout;
    active_contour_layout->addWidget(color_groupbox);
    active_contour_layout->addWidget(step_checkbox);

    active_contour_groupbox->setLayout(active_contour_layout);

    QGroupBox* tracking_display_groupbox = new QGroupBox(tr("Video tracking"));

    fps_checkbox = new QCheckBox(tr("show frames per second (FPS)"));
    mirrored_checkbox = new QCheckBox(tr("show mirrored (horizontally flipped) frames"));

    QVBoxLayout* tracking_display_layout = new QVBoxLayout;
    tracking_display_layout->addWidget(fps_checkbox);
    tracking_display_layout->addWidget(mirrored_checkbox);
    tracking_display_groupbox->setLayout(tracking_display_layout);


    ////////////////////////////////////////////

    QVBoxLayout* display_layout = new QVBoxLayout;
    display_layout->addWidget(size_groupbox);
    display_layout->addWidget(active_contour_groupbox);
    display_layout->addWidget(tracking_display_groupbox);
    display_layout->addStretch(1);

    page4 = new QWidget;
    page4->setLayout( display_layout );
}

void SettingsWindow::setupConnections()
{
    //connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_visu(int)) );

    connect( dial_buttons, SIGNAL(accepted()), this, SLOT(accept()) );
    connect( dial_buttons, SIGNAL(rejected()), this, SLOT(reject()) );
    connect( dial_buttons->button(QDialogButtonBox::Reset),
            SIGNAL(clicked()), this, SLOT(default_settings()) );

    //connect(klength_gradient_spin,SIGNAL(valueChanged(int)),this,
    //SLOT(show_phi_with_filtered_image()));

    //connect(alpha_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(beta_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    //connect(gamma_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    //connect(chanvese_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    //connect(geodesic_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

    connect(width_slider,      &QSlider::valueChanged,
            width_shape_spin,  &QSpinBox::setValue);

    connect(width_shape_spin,  QOverload<int>::of(&QSpinBox::valueChanged),
            width_slider,      &QSlider::setValue);

    connect(height_slider,      &QSlider::valueChanged,
            height_shape_spin,  &QSpinBox::setValue);

    connect(height_shape_spin,  QOverload<int>::of(&QSpinBox::valueChanged),
            height_slider,      &QSlider::setValue);

    connect(ordinate_slider,      &QSlider::valueChanged,
            ordinate_spin,  &QSpinBox::setValue);

    connect(ordinate_spin,  QOverload<int>::of(&QSpinBox::valueChanged),
            ordinate_slider,      &QSlider::setValue);

    connect(abscissa_slider,      &QSlider::valueChanged,
            abscissa_spin,  &QSpinBox::setValue);

    connect(abscissa_spin,  QOverload<int>::of(&QSpinBox::valueChanged),
            abscissa_slider,      &QSlider::setValue);





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

        //connect(page3,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

        //connect(histo_checkbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

    connect(outsidecolor_select,SIGNAL(clicked()),this,SLOT(set_color_out()));
    connect(insidecolor_select,SIGNAL(clicked()),this,SLOT(set_color_in()));


    connect(add_button, &QPushButton::clicked,
            this, &SettingsWindow::onAddShape);

    connect(subtract_button, &QPushButton::clicked,
            this, &SettingsWindow::onSubtractShape);

    connect(clear_button, &QPushButton::clicked,
            phiEditor_, &PhiEditor::clear);

    connect(phiViewModel_,
            &PhiViewModel::viewChanged,
            settingsView,
            &ImageView::displayImage);

    auto updateOverlay = [this]()
    {
        phiViewModel_->setOverlay( computeShapeInfo() );
    };

    connect(abscissa_spin,     &QSpinBox::valueChanged, this, updateOverlay);
    connect(ordinate_spin,     &QSpinBox::valueChanged, this, updateOverlay);
    connect(width_shape_spin,  &QSpinBox::valueChanged, this, updateOverlay);
    connect(height_shape_spin, &QSpinBox::valueChanged, this, updateOverlay);
    connect(rectangle_radio,   &QRadioButton::toggled, this,  updateOverlay);
    connect(ellipse_radio,     &QRadioButton::toggled, this,  updateOverlay);
}

ShapeInfo SettingsWindow::computeShapeInfo()
{
    ShapeInfo info;

    // --- Type de shape ---
    info.type = rectangle_radio->isChecked()
                    ? ShapeType::Rectangle
                    : ShapeType::Ellipse;

    const int canvasWidth = phiEditor_->phi().width();
    const int canvasHeight = phiEditor_->phi().height();

    // Récupération des valeurs des sliders (en pourcentage)
    float centerXPercent = abscissa_spin->value() / 100.0f; // De -500 à +500, donc normalisé autour de 0
    float centerYPercent = ordinate_spin->value() / 100.0f;

    // Calcul de la position du centre en pixels
    float centerX = (centerXPercent + 0.5f) * static_cast<float>(canvasWidth);
    float centerY = (centerYPercent + 0.5f) * static_cast<float>(canvasHeight);

    // Récupération des dimensions de la shape en pixels
    float width = width_shape_spin->value() / 100.0f * static_cast<float>(canvasWidth);
    float height = height_shape_spin->value() / 100.0f * static_cast<float>(canvasHeight);

    // Calcul de la bounding box à partir du centre et des dimensions
    float topLeftX = centerX - width / 2.0f;
    float topLeftY = centerY - height / 2.0f;

    info.boundingBox = QRect(topLeftX, topLeftY, width, height);

    return info;
}

void SettingsWindow::onAddShape()
{
    applyCurrentShape(true);   // true = add
}

void SettingsWindow::onSubtractShape()
{
    applyCurrentShape(false);  // false = subtract
}

void SettingsWindow::applyCurrentShape(bool add)
{
    if (!phiEditor_)
        return;

    if (add)
        phiEditor_->addShape( computeShapeInfo() );
    else
        phiEditor_->subtractShape( computeShapeInfo() );
}

void SettingsWindow::accept()
{
    auto& config = AppSettings::instance();

    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

    config.connectivity = ofeli_ip::Connectivity( connectivity_cb->currentIndex() );

    if( chanvese_radio->isChecked() )
    {
        config.speed = SpeedModel::REGION_BASED;
    }
    if( geodesic_radio->isChecked() )
    {
        config.speed = SpeedModel::EDGE_BASED;
    }

    //config.connectivity

    config.algo_config.is_cycle2 = internalspeed_groupbox->isChecked();
    config.algo_config.disk_radius = disk_radius_spin->value();
    config.algo_config.Na = Na_spin->value();
    config.algo_config.Ns = Ns_spin->value();

    config.region_ac_config.lambda_in = lambda_in_spin->value();
    config.region_ac_config.lambda_out = lambda_out_spin->value();

    config.region_ac_config.color_space = ofeli_ip::ColorSpaceOption( color_space_cb->currentIndex() );

    config.region_ac_config.weights.c1 = alpha_spin->value();
    config.region_ac_config.weights.c2 = beta_spin->value();
    config.region_ac_config.weights.c3 = gamma_spin->value();

    config.kernel_gradient_length = klength_gradient_spin->value();


    if ( downscale_factor_cb->currentIndex() == 0 )
    {
        config.downscale_factor = 1;
    }
    else if ( downscale_factor_cb->currentIndex() == 1 )
    {
        config.downscale_factor = 2;
    }
    else if ( downscale_factor_cb->currentIndex() == 2 )
    {
        config.downscale_factor = 4;
    }
    config.cycles_nbr = cycles_nbr_sb->value();



    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    phiEditor_->accept();

    ///////////////////////////////////
    //        Preprocessing          //
    ///////////////////////////////////

    config.has_preprocess = page3->isChecked();
    config.has_gaussian_noise = gaussian_noise_groupbox->isChecked();
    config.std_noise = float( std_noise_spin->value() );
    config.has_salt_noise = salt_noise_groupbox->isChecked();
    config.proba_noise = float( proba_noise_spin->value() ) / 100.f;
    config.has_speckle_noise = speckle_noise_groupbox->isChecked();
    config.std_speckle_noise = float( std_speckle_noise_spin->value() );
    config.has_median_filt = median_groupbox->isChecked();
    config.kernel_median_length = klength_median_spin->value();
    config.has_O1_algo = complex_radio2->isChecked();

    config.has_mean_filt = mean_groupbox->isChecked();
    config.kernel_mean_length = klength_mean_spin->value();

    config.has_gaussian_filt = gaussian_groupbox->isChecked();
    config.kernel_gaussian_length = klength_gaussian_spin->value();
    config.sigma = float( std_filter_spin->value() );
    config.has_aniso_diff = aniso_groupbox->isChecked();

    config.max_itera = iteration_filter_spin->value();
    config.lambda = float( lambda_spin->value() );
    config.kappa = float( kappa_spin->value() );
    if( aniso1_radio->isChecked() )
    {
        config.aniso_option = ofeli_ip::AnisoDiff::FUNCTION1;
    }
    else if( aniso2_radio->isChecked() )
    {
        config.aniso_option = ofeli_ip::AnisoDiff::FUNCTION2;
    }

    config.has_open_filt = open_groupbox->isChecked();
    config.kernel_open_length = klength_open_spin->value();
    config.has_close_filt = close_groupbox->isChecked();
    config.kernel_close_length = klength_close_spin->value();
    config.has_top_hat_filt = tophat_groupbox->isChecked();
    config.is_white_top_hat = whitetophat_radio->isChecked();
    config.kernel_tophat_length = klength_tophat_spin->value();
    config.has_O1_morpho = complex2_morpho_radio->isChecked();

    /////////////////////////////
    //        Display          //
    /////////////////////////////

    config.has_histo_normaliz = histo_checkbox->isChecked();
    config.has_display_each = step_checkbox->isChecked();

    config.inside_combo = insidecolor_combobox->currentIndex();
    config.outside_combo = outsidecolor_combobox->currentIndex();

    config.selected_in  = selected_in_disp;
    config.selected_out = selected_out_disp;


    if( config.outside_combo == ComboBoxColorIndex::SELECTED )
    {
        config.color_out = config.selected_out;
    }
    else
    {
       get_color(config.outside_combo,
                 config.color_out);
    }

    if( config.inside_combo == ComboBoxColorIndex::SELECTED )
    {
        config.color_in = config.selected_in;
    }
    else
    {
        get_color(config.inside_combo,
                  config.color_in);
    }

    config.is_show_fps = fps_checkbox->isChecked();
    config.is_show_mirrored = mirrored_checkbox->isChecked();

    // for data persistence
    config.save();

    QDialog::accept();
}

// si on clique sur le boutton Cancel de settings_window
// les widgets reprennent leur état, correspondant aux valeurs des paramètres
void SettingsWindow::reject()
{
    const auto& config = AppSettings::instance();

    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

    if ( config.connectivity == ofeli_ip::Connectivity::Four )
    {
        connectivity_cb->setCurrentIndex( 0 );
    }
    else if ( config.connectivity == ofeli_ip::Connectivity::Eight )
    {
        connectivity_cb->setCurrentIndex( 1 );
    }

    Na_spin->setValue(config.algo_config.Na);

    if( config.speed == SpeedModel::REGION_BASED )
    {
        chanvese_radio->setChecked(true);
    }
    else if( config.speed == SpeedModel::EDGE_BASED )
    {
        geodesic_radio->setChecked(true);
    }

    lambda_in_spin->setValue(config.region_ac_config.lambda_in);
    lambda_out_spin->setValue(config.region_ac_config.lambda_out);

    color_space_cb->setCurrentIndex( int(config.region_ac_config.color_space) );

    alpha_spin->setValue(config.region_ac_config.weights.c1);
    beta_spin->setValue(config.region_ac_config.weights.c2);
    gamma_spin->setValue(config.region_ac_config.weights.c3);

    klength_gradient_spin->setValue(config.kernel_gradient_length);

    if ( config.downscale_factor == 1 )
    {
        downscale_factor_cb->setCurrentIndex(0);
    }
    else if ( config.downscale_factor == 2 )
    {
        downscale_factor_cb->setCurrentIndex(1);
    }
    else if ( config.downscale_factor == 4 )
    {
        downscale_factor_cb->setCurrentIndex(2);
    }

    cycles_nbr_sb->setValue(config.cycles_nbr);


    Ns_spin->setValue(config.algo_config.Ns);
    internalspeed_groupbox->setChecked(config.algo_config.is_cycle2);
    disk_radius_spin->setValue(config.algo_config.disk_radius);

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    phiEditor_->reject();

    ///////////////////////////////////
    //        Preprocessing          //
    ///////////////////////////////////

    page3->setChecked(config.has_preprocess);

    gaussian_noise_groupbox->setChecked(config.has_gaussian_noise);
    std_noise_spin->setValue( double( config.std_noise ) );
    salt_noise_groupbox->setChecked(config.has_salt_noise);
    proba_noise_spin->setValue( double( 100.f*config.proba_noise ) );
    speckle_noise_groupbox->setChecked(config.has_speckle_noise);
    std_speckle_noise_spin->setValue( double( config.std_speckle_noise ) );

    median_groupbox->setChecked(config.has_median_filt);
    klength_median_spin->setValue(config.kernel_median_length);

    if( config.has_O1_algo )
    {
        complex_radio2->setChecked(true);
    }
    else
    {
        complex_radio1->setChecked(true);
    }

    mean_groupbox->setChecked(config.has_mean_filt);
    klength_mean_spin->setValue(config.kernel_mean_length);

    gaussian_groupbox->setChecked(config.has_gaussian_filt);
    klength_gaussian_spin->setValue(config.kernel_gaussian_length);
    std_filter_spin->setValue( double( config.sigma ) );

    aniso_groupbox->setChecked(config.has_aniso_diff);
    iteration_filter_spin->setValue(config.max_itera);
    lambda_spin->setValue( double( config.lambda ) );
    kappa_spin->setValue( double( config.kappa ) );
    if( config.aniso_option == ofeli_ip::AnisoDiff::FUNCTION1 )
    {
        aniso1_radio->setChecked(true);
    }
    else if( config.aniso_option == ofeli_ip::AnisoDiff::FUNCTION2 )
    {
        aniso2_radio->setChecked(true);
    }

    open_groupbox->setChecked(config.has_open_filt);
    klength_open_spin->setValue(config.kernel_open_length);

    close_groupbox->setChecked(config.has_close_filt);
    klength_close_spin->setValue(config.kernel_close_length);

    tophat_groupbox->setChecked(config.has_top_hat_filt);

    if( config.is_white_top_hat )
    {
        whitetophat_radio->setChecked(true);
    }
    else
    {
        blacktophat_radio->setChecked(true);
    }
    klength_tophat_spin->setValue(config.kernel_tophat_length);

    if( config.has_O1_morpho )
    {
        complex2_morpho_radio->setChecked(true);
    }
    else
    {
        complex1_morpho_radio->setChecked(true);
    }

    /////////////////////////////
    //        Display          //
    /////////////////////////////

    histo_checkbox->setChecked(config.has_histo_normaliz);
    step_checkbox->setChecked(config.has_display_each);

    insidecolor_combobox->setCurrentIndex(config.inside_combo);
    outsidecolor_combobox->setCurrentIndex(config.outside_combo);

    selected_in_disp = config.selected_in;
    selected_out_disp = config.selected_out;

    RgbColor color;

    if( config.inside_combo == ComboBoxColorIndex::SELECTED )
    {
        color = selected_in_disp;
    }
    else
    {
        get_color(config.inside_combo,
                  color);
    }
    QPixmap pm1(12,12);
    pm1.fill( QColor( get_QRgb(color) ) );
    insidecolor_combobox->setItemIcon(ComboBoxColorIndex::SELECTED,pm1);

    if( config.outside_combo == ComboBoxColorIndex::SELECTED )
    {
        color = selected_out_disp;
    }
    else
    {
        get_color(config.outside_combo,
                  color);
    }
    QPixmap pm2(12,12);
    pm2.fill( QColor( get_QRgb(color) ) );
    outsidecolor_combobox->setItemIcon(ComboBoxColorIndex::SELECTED,pm2);


    fps_checkbox->setChecked(config.is_show_fps);
    mirrored_checkbox->setChecked(config.is_show_mirrored);

    //calculate_filtered_copy_visu_buffers();
    //tab_visu( tabs->currentIndex() );

    phiEditor_->reject();

    QDialog::reject();
}

void SettingsWindow::default_settings()
{
    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

    connectivity_cb->setCurrentIndex( int(ofeli_ip::Connectivity::Four) );

    Na_spin->setValue( 30 );
    // region based model by default
    chanvese_radio->setChecked( true );
    lambda_in_spin->setValue( 1 );
    lambda_out_spin->setValue( 1 );

    color_space_cb->setCurrentIndex( int(ofeli_ip::ColorSpaceOption::RGB) );

    alpha_spin->setValue( 1 );
    beta_spin->setValue( 1 );
    gamma_spin->setValue( 1 );

    klength_gradient_spin->setValue( 5 );

    // /4
    downscale_factor_cb->setCurrentIndex( 2 );
    cycles_nbr_sb->setValue( 3 );

    internalspeed_groupbox->setChecked( true );
    Ns_spin->setValue( 3 );
    disk_radius_spin->setValue( 2 );

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    ellipse_radio->setChecked( true );

    // 65% width and 65% height by default
    width_shape_spin->setValue(65);
    height_shape_spin->setValue(65);

    // centered by default
    abscissa_spin->setValue(0);
    ordinate_spin->setValue(0);

    ///////////////////////////////////
    //        Preprocessing          //
    ///////////////////////////////////

    page3->setChecked( false );

    gaussian_noise_groupbox->setChecked( false );
    std_noise_spin->setValue( 20.0 );

    salt_noise_groupbox->setChecked( false );
    proba_noise_spin->setValue( 0.05 );

    speckle_noise_groupbox->setChecked(false);
    std_speckle_noise_spin->setValue( 0.16 );

    median_groupbox->setChecked( false );
    klength_median_spin->setValue( 5 );

    // has_O1_algo
    complex_radio2->setChecked( true );

    mean_groupbox->setChecked( false );
    klength_mean_spin->setValue( 5 );

    gaussian_groupbox->setChecked( false );
    klength_gaussian_spin->setValue( 5 );
    std_filter_spin->setValue( 2.0 );

    aniso_groupbox->setChecked( false );
    // AnisoDiff::FUNCTION1
    aniso1_radio->setChecked( true );
    iteration_filter_spin->setValue( 10 );
    lambda_spin->setValue( 1.0/7.0 );
    kappa_spin->setValue( 30.0 );

    open_groupbox->setChecked( false );
    klength_open_spin->setValue( 5 );

    close_groupbox->setChecked( false );
    klength_close_spin->setValue( 5 );

    tophat_groupbox->setChecked( false );
    whitetophat_radio->setChecked( true );
    klength_tophat_spin->setValue( 5 );

    // config.has_O1_morpho
    complex2_morpho_radio->setChecked( true );

    /////////////////////////////
    //        Display          //
    /////////////////////////////

    histo_checkbox->setChecked( true );
    step_checkbox->setChecked( true );

    outsidecolor_combobox->setCurrentIndex( ComboBoxColorIndex::BLUE );
    insidecolor_combobox->setCurrentIndex( ComboBoxColorIndex::RED );

    fps_checkbox->setChecked( true );
    mirrored_checkbox->setChecked( true );

    //calculate_filtered_copy_visu_buffers();
    //tab_visu( tabs->currentIndex() );
}


// Fonction appelée par le boutton selected de boundaries outside
// sélection et affichage d'une couleur particulière
void SettingsWindow::set_color_out()
{
    // Sélection d'une QColor a partir d'une boite de dialogue couleur
    QColor color_out = QColorDialog::getColor(Qt::white, this, tr("Select Lout color"));
    if( color_out.isValid() )
    {
        selected_out_disp.red   = (unsigned char)(color_out.red());
        selected_out_disp.green = (unsigned char)(color_out.green());
        selected_out_disp.blue  = (unsigned char)(color_out.blue());

        QPixmap pm(12,12);
        pm.fill(color_out);
        outsidecolor_combobox->setItemIcon(ComboBoxColorIndex::SELECTED,pm);

        outsidecolor_combobox->setCurrentIndex(ComboBoxColorIndex::SELECTED);
    }
}

// Fonction appelée par le boutton selected de boundaries inside
// sélection et affichage d'une couleure particulière
void SettingsWindow::set_color_in()
{
    // Selection d'une QColor à partir d'une boîte de dialogue couleur
    QColor color_in = QColorDialog::getColor(Qt::white, this, tr("Select Lin color"));
    if( color_in.isValid() )
    {
        selected_in_disp.red   = (unsigned char)(color_in.red());
        selected_in_disp.green = (unsigned char)(color_in.green());
        selected_in_disp.blue  = (unsigned char)(color_in.blue());

        QPixmap pm(12,12);
        pm.fill(color_in);
        insidecolor_combobox->setItemIcon(ComboBoxColorIndex::SELECTED,pm);

        insidecolor_combobox->setCurrentIndex(ComboBoxColorIndex::SELECTED);
    }
}

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
}

void SettingsWindow::showEvent(QShowEvent* event)
{
    if ( phiViewModel_ != nullptr )
    {
        phiViewModel_->setOverlay( computeShapeInfo() );
    }
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue( "Settings/Window/geometry", saveGeometry() );

    QDialog::closeEvent(event);
}

}
