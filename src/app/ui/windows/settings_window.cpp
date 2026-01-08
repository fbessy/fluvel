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

#include <QtWidgets>

#include <stack>
#include <ctime>         // for std::clock_t, std::clock() and CLOCKS_PER_SEC
#include <cstring>       // for std::memcpy

namespace ofeli_gui
{

SettingsWindow::SettingsWindow(QWidget* parent) :
    QDialog(parent),
    scale_spin(nullptr),
    scale_slider(nullptr),
    filters2(nullptr),
    img2_filtered(nullptr),
    phi2(nullptr),
    displayed_phi_shape(nullptr),
    image_filter_uchar(nullptr),
    image_phi_uchar(nullptr),
    image_shape_uchar(nullptr),
    img1(nullptr)
{
    setWindowTitle(tr("Settings"));

    QSettings settings;

    const auto geo = settings.value("Settings/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    scrollArea_settings = new ScrollAreaWidget(this);
    imageLabel_settings = new PixmapWidget(scrollArea_settings);
    imageLabel_settings->setBackgroundRole(QPalette::Dark);
    imageLabel_settings->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel_settings->setScaledContents(true);
    imageLabel_settings->setMouseTracking(true);
    imageLabel_settings->installEventFilter(this);
    imageLabel_settings->setAcceptDrops(true);
    imageLabel_settings->setAlignment(Qt::AlignCenter);
    QString text(tr("<drag ϕ(t=0)>"));
    imageLabel_settings->set_text(text);
    imageLabel_settings->resize(200,200);
    scrollArea_settings->setBackgroundRole(QPalette::Dark);
    scrollArea_settings->setWidget(imageLabel_settings);
    scrollArea_settings->setAlignment(Qt::AlignCenter);
    scrollArea_settings->setWidgetResizable(true);
    connect(scrollArea_settings->verticalScrollBar(),
            SIGNAL(rangeChanged(int,int)),this,
            SLOT(adjustVerticalScroll_settings (int,int)));
    connect(scrollArea_settings->horizontalScrollBar(),
            SIGNAL(rangeChanged(int,int)),this,
            SLOT(adjustHorizontalScroll_settings(int,int)));

    dial_buttons = new QDialogButtonBox(this);
    dial_buttons->addButton(QDialogButtonBox::Ok);
    dial_buttons->addButton(QDialogButtonBox::Cancel);
    dial_buttons->addButton(QDialogButtonBox::Reset);
    connect( dial_buttons, SIGNAL(accepted()), this, SLOT(accept()) );
    connect( dial_buttons, SIGNAL(rejected()), this, SLOT(reject()) );
    connect( dial_buttons->button(QDialogButtonBox::Reset),
             SIGNAL(clicked()), this, SLOT(default_settings()) );

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Algorithm tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* externalspeed_groupbox = new QGroupBox(tr("Cycle 1 : data dependant evolution"));

    Na_spin = new QSpinBox;
    Na_spin->setSingleStep(1);
    Na_spin->setMinimum(1);
    Na_spin->setMaximum(999);
    Na_spin->setSuffix(tr(" iterations"));
    Na_spin->setToolTip(tr("iterations in the cycle 1, active contour penetrability"));
    QFormLayout *Na_layout = new QFormLayout;
    Na_layout->addRow("Na =", Na_spin);

#ifdef Q_OS_MAC
    Na_layout->setFormAlignment(Qt::AlignLeft);
#endif


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

#ifdef Q_OS_MAC
    lambda_layout->setFormAlignment(Qt::AlignLeft);
#endif

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addWidget(chanvese_radio);
    chanvese_layout->addLayout(lambda_layout);

    geodesic_radio = new QRadioButton(tr("geodesic model"));
    geodesic_radio->setToolTip(tr("edge-based model for smoothed multimodal images"));

    klength_gradient_spin = new KernelSizeSpinBox;
    klength_gradient_spin->setSingleStep(2);
    klength_gradient_spin->setMinimum(3);
    klength_gradient_spin->setMaximum(499);
    //klength_gradient_spin->setSuffix("²");
    klength_gradient_spin->setToolTip(tr("morphological gradient structuring element size"));
    QFormLayout *gradient_layout = new QFormLayout;
    gradient_layout->addRow(tr("SE size ="), klength_gradient_spin);

#ifdef Q_OS_MAC
    gradient_layout->setFormAlignment(Qt::AlignLeft);
#endif

    connect(klength_gradient_spin,SIGNAL(valueChanged(int)),this,
            SLOT(show_phi_with_filtered_image()));

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

    connect(alpha_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    connect(beta_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    connect(gamma_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));

    QFormLayout* color_weights_layout = new QFormLayout;
    color_weights_layout->addRow(tr("1st component weight ="), alpha_spin);
    color_weights_layout->addRow(tr("2nd component weight ="), beta_spin);
    color_weights_layout->addRow(tr("3rd component weight ="), gamma_spin);

#ifdef Q_OS_MAC
    color_weights_layout->setFormAlignment(Qt::AlignLeft);
#endif

    QVBoxLayout* vcolor_layout = new QVBoxLayout;
    vcolor_layout->addWidget(color_space_cb);
    vcolor_layout->addLayout(color_weights_layout);

    color_weights_groupbox->setLayout(vcolor_layout);

    connect(chanvese_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(geodesic_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

    QVBoxLayout* externalspeed_layout = new QVBoxLayout;

    externalspeed_layout->addLayout(Na_layout);
    externalspeed_layout->addLayout(speed_layout);
    externalspeed_layout->addWidget(color_weights_groupbox);
    externalspeed_groupbox->setLayout(externalspeed_layout);

    ////////////////////////////////////////////

    internalspeed_groupbox = new QGroupBox(tr("Cycle 2 - smoothing via gaussian filtring"));
    internalspeed_groupbox->setCheckable(true);
    internalspeed_groupbox->setChecked(true);

    Ns_spin = new QSpinBox;
    Ns_spin->setSingleStep(1);
    Ns_spin->setMinimum(1);
    Ns_spin->setMaximum(999);
    Ns_spin->setSuffix(tr(" iterations"));
    Ns_spin->setToolTip(tr("iterations in the cycle 2, active contour regularization"));

    klength_spin = new KernelSizeSpinBox;
    klength_spin->setSingleStep(2);
    klength_spin->setMinimum(3);
    klength_spin->setMaximum(499);
    klength_spin->setToolTip(tr("gaussian kernel size = Ng × Ng "));
    std_spin = new QDoubleSpinBox;
    std_spin->setSingleStep(0.1);
    std_spin->setMinimum(0.0);
    std_spin->setMaximum(1000000.0);
    std_spin->setToolTip(tr("standard deviation of the gaussian kernel"));

    QFormLayout* internalspeed_layout = new QFormLayout;
    internalspeed_layout->addRow("Ns =", Ns_spin);
    internalspeed_layout->addRow("Ng =", klength_spin);
    internalspeed_layout->addRow("σ =", std_spin);

#ifdef Q_OS_MAC
    internalspeed_layout->setFormAlignment(Qt::AlignLeft);
#endif

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
    algorithm_layout->addWidget(externalspeed_groupbox);
    algorithm_layout->addWidget(internalspeed_groupbox);
    algorithm_layout->addWidget(tracking_groupbox);
    algorithm_layout->addStretch(1);

    QWidget* page1 = new QWidget;
    page1->setLayout(algorithm_layout);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialization tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    open_phi_button = new QPushButton(tr("Open ϕ(t=0)"));
    open_phi_button->setToolTip(tr("or drag and drop an image of ϕ(t=0)"));
    save_phi_button = new QPushButton(tr("Save ϕ(t=0)"));
    connect(open_phi_button,SIGNAL(clicked()),this,SLOT(openFilenamePhi()));
    connect(save_phi_button,SIGNAL(clicked()),this,SLOT(save_phi()));
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
    connect(rectangle_radio,SIGNAL(clicked()),this,SLOT(shape_visu()));
    connect(ellipse_radio,SIGNAL(clicked()),this,SLOT(shape_visu()));

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
    width_shape_spin->setMaximum(1000);
    width_shape_spin->setSuffix(tr(" % image width"));

    QFormLayout* width_spin_layout = new QFormLayout;
    width_spin_layout->addRow(tr("width ="), width_shape_spin);

#ifdef Q_OS_MAC
    width_spin_layout->setFormAlignment(Qt::AlignLeft);
#endif

    width_slider = new QSlider(Qt::Horizontal, this);

#ifdef Q_OS_LINUX
    width_slider->setTickPosition(QSlider::TicksBelow);
#else
    width_slider->setTickPosition(QSlider::TicksAbove);
#endif

    width_slider->setMinimum(0);
    width_slider->setMaximum(150);
    width_slider->setTickInterval(25);
    width_slider->setSingleStep(15);

    connect(width_shape_spin,SIGNAL(valueChanged(int)),this,
            SLOT(shape_visu(int)));
    connect(width_slider,SIGNAL(valueChanged(int)),width_shape_spin,
            SLOT(setValue(int)));
    connect(width_shape_spin,SIGNAL(valueChanged(int)),width_slider,
            SLOT(setValue(int)));

    height_shape_spin = new QSpinBox;
    height_shape_spin->setSingleStep(15);
    height_shape_spin->setMinimum(0);
    height_shape_spin->setMaximum(1000);
    height_shape_spin->setSuffix(tr((" % image height")));

    QFormLayout* height_spin_layout = new QFormLayout;
    height_spin_layout->addRow(tr("height ="), height_shape_spin);

#ifdef Q_OS_MAC
    height_spin_layout->setFormAlignment(Qt::AlignLeft);
#endif

    height_slider = new QSlider(Qt::Horizontal, this);

#ifdef Q_OS_LINUX
    height_slider->setTickPosition(QSlider::TicksBelow);
#else
    height_slider->setTickPosition(QSlider::TicksAbove);
#endif

    height_slider->setMinimum(0);
    height_slider->setMaximum(150);
    height_slider->setTickInterval(25);
    height_slider->setSingleStep(15);

    connect(height_shape_spin,SIGNAL(valueChanged(int)),this,
            SLOT(shape_visu(int)));
    connect(height_slider,SIGNAL(valueChanged(int)),height_shape_spin,
            SLOT(setValue(int)));
    connect(height_shape_spin,SIGNAL(valueChanged(int)),height_slider,
            SLOT(setValue(int)));

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
    abscissa_spin->setMinimum(-500);
    abscissa_spin->setMaximum(500);
    abscissa_spin->setSuffix(tr(" % image width"));

    QFormLayout* abscissa_spin_layout = new QFormLayout;
    abscissa_spin_layout->addRow("x = Xo +", abscissa_spin);

#ifdef Q_OS_MAC
    abscissa_spin_layout->setFormAlignment(Qt::AlignLeft);
#endif

    abscissa_slider = new QSlider(Qt::Horizontal, this);

#ifdef Q_OS_LINUX
    abscissa_slider->setTickPosition(QSlider::TicksBelow);
#else
    abscissa_slider->setTickPosition(QSlider::TicksAbove);
#endif

    abscissa_slider->setMinimum(-75);
    abscissa_slider->setMaximum(75);
    abscissa_slider->setTickInterval(25);
    abscissa_slider->setSingleStep(15);

    connect(abscissa_spin,SIGNAL(valueChanged(int)),this,SLOT(shape_visu(int)));
    connect(abscissa_slider,SIGNAL(valueChanged(int)),abscissa_spin,SLOT(setValue(int)));
    connect(abscissa_spin,SIGNAL(valueChanged(int)),abscissa_slider,SLOT(setValue(int)));

    ordinate_spin = new QSpinBox;
    ordinate_spin->setSingleStep(15);
    ordinate_spin->setMinimum(-500);
    ordinate_spin->setMaximum(500);
    ordinate_spin->setSuffix(tr(" % image height"));

    QFormLayout* ordinate_spin_layout = new QFormLayout;
    ordinate_spin_layout->addRow("y = Yo +", ordinate_spin);

#ifdef Q_OS_MAC
    ordinate_spin_layout->setFormAlignment(Qt::AlignLeft);
#endif

    ordinate_slider = new QSlider(Qt::Horizontal, this);

#ifdef Q_OS_LINUX
    ordinate_slider->setTickPosition(QSlider::TicksBelow);
#else
    ordinate_slider->setTickPosition(QSlider::TicksAbove);
#endif

    ordinate_slider->setMinimum(-75);
    ordinate_slider->setMaximum(75);
    ordinate_slider->setTickInterval(25);
    ordinate_slider->setSingleStep(15);

    connect(ordinate_spin,SIGNAL(valueChanged(int)),this,SLOT(shape_visu(int)));
    connect(ordinate_slider,SIGNAL(valueChanged(int)),ordinate_spin,SLOT(setValue(int)));
    connect(ordinate_spin,SIGNAL(valueChanged(int)),ordinate_slider,SLOT(setValue(int)));

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
    connect(add_button,SIGNAL(clicked()),this,SLOT(add_visu()));

    subtract_button = new QPushButton(tr("Subtract"));
    subtract_button->setToolTip(tr("or click on the right mouse button when the cursor is in the image"));
    connect(subtract_button,SIGNAL(clicked()),this,SLOT(subtract_visu()));

    clean_button = new QPushButton(tr("Clean"));
    connect(clean_button,SIGNAL(clicked()),this,SLOT(clean_phi_visu()));

    add_button->setEnabled(false);
    subtract_button->setEnabled(false);
    clean_button->setEnabled(false);

    QHBoxLayout* modify_layout = new QHBoxLayout;
    modify_layout->addWidget(clean_button);
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
    page2->setEnabled(false);

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
    connect(gaussian_noise_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(std_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

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
    connect(salt_noise_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(proba_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

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
    connect(speckle_noise_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(std_speckle_noise_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));

    QFormLayout* speckle_noise_layout = new QFormLayout;
    speckle_noise_layout->addRow("σ =", std_speckle_noise_spin);

#ifdef Q_OS_MAC
    gaussian_noise_layout->setFormAlignment(Qt::AlignLeft);
    salt_noise_layout->setFormAlignment(Qt::AlignLeft);
    speckle_noise_layout->setFormAlignment(Qt::AlignLeft);
#endif

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
    //klength_mean_spin->setSuffix("²");

    connect(mean_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(klength_mean_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    QFormLayout* mean_layout = new QFormLayout;
    mean_layout->addRow(tr("kernel size ="), klength_mean_spin);

    gaussian_groupbox = new QGroupBox(tr("Gaussian filter"));
    gaussian_groupbox->setCheckable(true);
    gaussian_groupbox->setChecked(false);
    klength_gaussian_spin = new KernelSizeSpinBox;
    klength_gaussian_spin->setSingleStep(2);
    klength_gaussian_spin->setMinimum(3);
    klength_gaussian_spin->setMaximum(499);
    //klength_gaussian_spin->setSuffix("²");
    std_filter_spin = new QDoubleSpinBox;
    std_filter_spin->setSingleStep(0.1);
    std_filter_spin->setMinimum(0.0);
    std_filter_spin->setMaximum(1000000.0);
    std_filter_spin->setToolTip(tr("standard deviation"));
    connect(gaussian_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(klength_gaussian_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    connect(std_filter_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));
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
    connect(median_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(klength_median_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    connect(complex_radio1,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(complex_radio2,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
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
    connect(aniso_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(aniso1_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(aniso2_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(iteration_filter_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    connect(lambda_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));
    connect(kappa_spin,SIGNAL(valueChanged(double)),this,SLOT(show_phi_with_filtered_image()));
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
    //klength_open_spin->setSuffix("²");
    klength_open_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));
    connect(open_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(klength_open_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
    QFormLayout* open_layout = new QFormLayout;
    open_layout->addRow(tr("SE size ="), klength_open_spin);

    close_groupbox = new QGroupBox(tr("Closing"));
    close_groupbox->setCheckable(true);
    close_groupbox->setChecked(false);
    klength_close_spin = new KernelSizeSpinBox;
    klength_close_spin->setSingleStep(2);
    klength_close_spin->setMinimum(3);
    klength_close_spin->setMaximum(499);
    //klength_close_spin->setSuffix("²");
    klength_close_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));
    connect(close_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(klength_close_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
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
    //klength_tophat_spin->setSuffix("²");
    klength_tophat_spin->setToolTip(tr("the structuring element shape is a square and its origin is the center of the square"));
    connect(tophat_groupbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(whitetophat_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(blacktophat_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(klength_tophat_spin,SIGNAL(valueChanged(int)),this,SLOT(show_phi_with_filtered_image()));
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
    connect(complex1_morpho_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));
    connect(complex2_morpho_radio,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

#ifdef Q_OS_MAC
    median_layout->setFormAlignment(Qt::AlignLeft);
    mean_layout->setFormAlignment(Qt::AlignLeft);
    gaussian_layout->setFormAlignment(Qt::AlignLeft);
    aniso_layout->setFormAlignment(Qt::AlignLeft);
    open_layout->setFormAlignment(Qt::AlignLeft);
    close_layout->setFormAlignment(Qt::AlignLeft);
    tophat_layout->setFormAlignment(Qt::AlignLeft);
#endif

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
    connect(page3,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

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

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Display tab
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGroupBox* size_groupbox = new QGroupBox(tr("Image"));

    scale_spin = new QSpinBox;
    scale_spin->setSingleStep(25);
    scale_spin->setMinimum(1);
    scale_spin->setMaximum(5000);
    scale_spin->setSuffix(" %");
    scale_spin->setValue(100);

    scale_slider = new QSlider(Qt::Horizontal, this);

#ifdef Q_OS_LINUX
    scale_slider->setTickPosition(QSlider::TicksBelow);
#else
    scale_slider->setTickPosition(QSlider::TicksAbove);
#endif

    scale_slider->setMinimum(1);
    scale_slider->setMaximum(1000);
    scale_slider->setTickInterval(100);
    scale_slider->setSingleStep(25);
    scale_slider->setValue(100);

    connect(scale_spin,SIGNAL(valueChanged(int)),this,SLOT(do_scale(int)));
    connect(scale_slider,SIGNAL(valueChanged(int)),scale_spin,SLOT(setValue(int)));
    connect(scale_spin,SIGNAL(valueChanged(int)),scale_slider,SLOT(setValue(int)));

    scale_spin->installEventFilter(this);
    scale_slider->installEventFilter(this);
    scale_spin->setMouseTracking(true);
    scale_slider->setMouseTracking(true);

    histo_checkbox = new QCheckBox(tr("histogram normalization for the gradient"));
    connect(histo_checkbox,SIGNAL(clicked()),this,SLOT(show_phi_with_filtered_image()));

    QFormLayout* size_layout = new QFormLayout;
    size_layout->addRow(tr("scale ="), scale_spin);
    size_layout->addRow(scale_slider);
    size_layout->addRow(" ",histo_checkbox);

#ifdef Q_OS_MAC
    size_layout->setFormAlignment(Qt::AlignLeft);
#endif

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


    connect(outsidecolor_combobox,SIGNAL(activated(int)),this,SLOT(phi_visu(int)));
    connect(insidecolor_combobox,SIGNAL(activated(int)),this,SLOT(phi_visu(int)));

    QPushButton* outsidecolor_select = new QPushButton(tr("Select"));
    connect(outsidecolor_select,SIGNAL(clicked()),this,SLOT(set_color_out()));

    QPushButton* insidecolor_select = new QPushButton(tr("Select"));
    connect(insidecolor_select,SIGNAL(clicked()),this,SLOT(set_color_in()));

    QFormLayout* Loutcolor_form = new QFormLayout;
    Loutcolor_form->addRow(tr("Lout :"), outsidecolor_combobox);

#ifdef Q_OS_MAC
    Loutcolor_form->setFormAlignment(Qt::AlignLeft);
#endif

    QFormLayout* Lincolor_form = new QFormLayout;
    Lincolor_form->addRow(tr("   Lin :"), insidecolor_combobox);

#ifdef Q_OS_MAC
    Lincolor_form->setFormAlignment(Qt::AlignLeft);
#endif

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

    QWidget* page4 = new QWidget;
    page4->setLayout( display_layout );


    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////

    tabs = new QTabWidget(this);
    tabs->addTab( page1, tr("Algorithm") );
    tabs->addTab( page2, tr("Initialization") );
    tabs->addTab( page3, tr("Preprocessing") );
    tabs->addTab( page4, tr("Display") );
    connect( tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_visu(int)) );

    QGridLayout *settings_grid = new QGridLayout;
    settings_grid->addWidget(tabs,0,0);
    settings_grid->addWidget(scrollArea_settings,0,1);
    settings_grid->addWidget(dial_buttons,1,1);

    settings_grid->setColumnStretch(1,1);
    setLayout(settings_grid);

    last_directory_used = settings.value("Main/Name/last_directory_used",QDir().homePath()).toString();

    has_contours_hidden = true;
    scale_spin->setValue(settings.value("Settings/Display/zoom_factor", 100).toInt());
    imageLabel_settings->set_zoomFactor(float(scale_spin->value())/100.0f);

    reject();
    scrollArea_settings->setFocus(Qt::OtherFocusReason);
}

void SettingsWindow::init(const unsigned char* img0,
                          int img0_width,
                          int img0_height,
                          bool is_rgb0,
                          const QImage& qimg0)
{
    if( img0 != nullptr &&
        img0_width > 0 &&
        img0_height > 0 &&
        !qimg0.isNull() )
    {
        imageLabel_settings->set_qimage0(qimg0);

        img1 = img0;
        img_width = img0_width;
        img_height = img0_height;
        img_size = img0_width*img0_height;
        is_rgb1 = is_rgb0;
        img = qimg0;

        if( displayed_phi_shape != nullptr )
        {
            delete displayed_phi_shape;
            displayed_phi_shape = nullptr;
        }
        if( displayed_phi_shape == nullptr )
        {
            displayed_phi_shape = new ofeli_ip::Matrix<signed char>(img_width,img_height);
        }

        auto& config = AppSettings::instance();

        if( config.Lout_init.empty() ||
            config.Lin_init.empty() ||
            config.phi_width  <= 0 ||
            config.phi_height <= 0 )
        {
            config.phi_width = img_width;
            config.phi_height = img_height;

            config.Lout_init.clear();
            config.Lin_init.clear();

            ofeli_ip::BoundaryBuilder lists_init( config.phi_width,
                                                  config.phi_height,
                                                  config.Lout_init,
                                                  config.Lin_init );

            lists_init.get_rectangle_points(0.05f, 0.05f, 0.95f, 0.95f);
        }
        else if( config.phi_width  != img_width ||
                 config.phi_height != img_height )
        {
            ofeli_ip::Matrix<signed char> phi_temp( config.phi_width,
                                                    config.phi_height );

            do_flood_fill_from_lists( config.Lout_init,
                                      config.Lin_init,
                                      phi_temp );

            QImage img_phi = QImage( (const unsigned char*)( phi_temp.get_matrix_data() ),
                                     phi_temp.get_width(),
                                     phi_temp.get_height(),
                                     phi_temp.get_width(),
                                     QImage::Format_Indexed8 );

            QVector<QRgb> table(256);
            for( int I = 0; I < 256; I++ )
            {
                table[I] = qRgb(I,I,I);
            }
            img_phi.setColorTable(table);

            QImage scaled_img_phi = img_phi.scaled( img_width,
                                                    img_height,
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::SmoothTransformation );

            scaled_img_phi = scaled_img_phi.convertToFormat(QImage::Format_RGB32);

            ofeli_ip::Matrix<signed char> scaled_phi( img_width,
                                                      img_height );

            QRgb pix;

            for( int y = 0; y < img_height; y++ )
            {
                for( int x = 0; x < img_width; x++ )
                {
                    pix = scaled_img_phi.pixel(x,y);

                    if( qRed(pix) > 127 )
                    {
                        scaled_phi(x,y) = ofeli_ip::PhiValue::INSIDE_REGION;
                    }
                    else
                    {
                        scaled_phi(x,y) = ofeli_ip::PhiValue::OUTSIDE_REGION;
                    }
                }
            }

            find_lists_from_phi( scaled_phi,
                                 config.Lout_init,
                                 config.Lin_init );

            config.phi_width = img_width;
            config.phi_height = img_height;
        }

        Lout33 = config.Lout_init;
        Lin33 = config.Lin_init;

        if( phi2 != nullptr )
        {
            delete phi2;
            phi2 = nullptr;
        }
        if( phi2 == nullptr )
        {
            phi2 = new ofeli_ip::Matrix<signed char>( config.phi_width,
                                                      config.phi_height );
        }

        do_flood_fill_from_lists(Lout33,Lin33,*phi2);

        if( filters2 != nullptr )
        {
            delete filters2;
            filters2 = nullptr;
        }

        if( filters2 == nullptr )
        {
            if( is_rgb1 )
            {
                filters2 = new ofeli_ip::Filters(img1,img_width,img_height,4);
            }
            else
            {
                filters2 = new ofeli_ip::Filters(img1,img_width,img_height,1);
            }
        }






        if( image_filter_uchar != nullptr )
        {
            delete[] image_filter_uchar;
            image_filter_uchar = nullptr;
        }
        if( image_filter_uchar == nullptr )
        {
            image_filter_uchar = new unsigned char[4*img_size];
        }
        image_filter = QImage(image_filter_uchar, img_width, img_height, 4*img_width, QImage::Format_RGB32);

        if( image_phi_uchar != nullptr )
        {
            delete[] image_phi_uchar;
            image_phi_uchar = nullptr;
        }
        if( image_phi_uchar == nullptr )
        {
            image_phi_uchar = new unsigned char[4*img_size];
        }
        image_phi = QImage(image_phi_uchar, img_width, img_height, 4*img_width, QImage::Format_RGB32);

        if( image_shape_uchar != nullptr )
        {
            delete[] image_shape_uchar;
            image_shape_uchar = nullptr;
        }
        if( image_shape_uchar == nullptr )
        {
            image_shape_uchar = new unsigned char[4*img_size];
        }
        image_shape = QImage(image_shape_uchar, img_width, img_height, 4*img_width, QImage::Format_RGB32);











        update_visu();

        disconnect(scale_spin,SIGNAL(valueChanged(int)),this,SLOT(do_scale(int)));

        scale_spin->setMaximum(1000000/img_height);
        scale_spin->setSingleStep(80000/(7*img_height));

        scale_slider->setMaximum(160000/img_height);
        scale_slider->setTickInterval(160000/(7*img_height));

        connect(scale_spin,SIGNAL(valueChanged(int)),this,SLOT(do_scale(int)));

        page2->setEnabled(true);
        open_phi_button->setEnabled(true);
        save_phi_button->setEnabled(true);
        add_button->setEnabled(true);
        subtract_button->setEnabled(true);
        clean_button->setEnabled(true);

        if( is_rgb1 )
        {
            color_weights_groupbox->setHidden(false);
        }
        else
        {
            color_weights_groupbox->setHidden(true);
        }
    }
    else
    {
        std::cerr << "Error." << std::endl;
    }
}

void SettingsWindow::update_visu()
{
    if( parent() != nullptr )
    {
        //scale_spin->setValue(static_cast<ImageWindow*>(parent())->get_zoom_factor());

        if( tabs->currentIndex() == TabIndex::INITIALIZATION )
        {
            calculate_filtered_copy_visu_buffers();
            phi_visu(true);
            shape_visu();
        }
        else
        {
            calculate_filtered_copy_visu_buffers();
            phi_visu(false);
        }
    }
}

void SettingsWindow::accept()
{
    auto& config = AppSettings::instance();

    bool is_ac_config_changed = false;

    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

    if( chanvese_radio->isChecked() )
    {
        config.speed = SpeedModel::REGION_BASED;
    }
    if( geodesic_radio->isChecked() )
    {
        config.speed = SpeedModel::EDGE_BASED;
    }

    ofeli_ip::AcConfig previous_algo_config = config.algo_config;

    config.algo_config.is_cycle2 = internalspeed_groupbox->isChecked();
    config.algo_config.kernel_length = klength_spin->value();
    config.algo_config.sigma = float( std_spin->value() );
    config.algo_config.Na = Na_spin->value();
    config.algo_config.Ns = Ns_spin->value();

    ofeli_ip::RegionColorConfig previous_region_config = config.region_ac_config;

    config.region_ac_config.lambda_in = lambda_in_spin->value();
    config.region_ac_config.lambda_out = lambda_out_spin->value();

    config.region_ac_config.color_space = ofeli_ip::ColorSpaceOption( color_space_cb->currentIndex() );

    config.region_ac_config.weights[0] = alpha_spin->value();
    config.region_ac_config.weights[1] = beta_spin->value();
    config.region_ac_config.weights[2] = gamma_spin->value();

    if(    previous_algo_config != config.algo_config
        || previous_region_config != config.region_ac_config )
    {
        is_ac_config_changed = true;
    }

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

    if( rectangle_radio->isChecked() )
    {
        config.has_ellipse = false;
    }
    if( ellipse_radio->isChecked() )
    {
        config.has_ellipse = true;
    }

    config.init_width = float(width_shape_spin->value())/100.f;
    config.init_height = float(height_shape_spin->value())/100.f;

    config.center_x = float(abscissa_spin->value())/100.f;
    config.center_y = float(ordinate_spin->value())/100.f;

    config.Lout_init = Lout33;
    config.Lin_init = Lin33;

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

    if( parent() != nullptr )
    {
        //static_cast<ImageWindow*>(parent())->set_zoom_factor(scale_spin->value());
    }

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

    alpha_spin->setValue(config.region_ac_config.weights[0]);
    beta_spin->setValue(config.region_ac_config.weights[1]);
    gamma_spin->setValue(config.region_ac_config.weights[2]);

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
    klength_spin->setValue(config.algo_config.kernel_length);
    std_spin->setValue( double( config.algo_config.sigma ) );

    ///////////////////////////////////
    //       Initialization          //
    ///////////////////////////////////

    if( !config.has_ellipse )
    {
        rectangle_radio->setChecked(true);
    }
    else
    {
        ellipse_radio->setChecked(true);
    }

    width_shape_spin->setValue(int(config.init_width*100.0));
    height_shape_spin->setValue(int(config.init_height*100.0));

    abscissa_spin->setValue(int(config.center_x*100.0));
    ordinate_spin->setValue(int(config.center_y*100.0));

    Lout33 = config.Lout_init;
    Lin33 = config.Lin_init;

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

    if( parent() != nullptr )
    {
        //scale_spin->setValue(static_cast<ImageWindow*>(parent())->get_zoom_factor());
    }

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

    calculate_filtered_copy_visu_buffers();
    tab_visu( tabs->currentIndex() );

    QDialog::reject();
}

void SettingsWindow::default_settings()
{
    ///////////////////////////////////
    //          Algorithm            //
    ///////////////////////////////////

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
    klength_spin->setValue( 5 );
    std_spin->setValue( 2.0 );

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

    calculate_filtered_copy_visu_buffers();
    tab_visu( tabs->currentIndex() );
}

// Fonction appelée pour le changement d 'echelle de l'image dans la fenêtre paramètre
void SettingsWindow::do_scale(int value)
{
    if( img1 != nullptr )
    {
        imageLabel_settings->setZoomFactor( float(value)/100.f );
    }
}

// Fonction appelée dans l'onglet initalisation pour calculer et afficher l'image+phi(couleur foncé)+forme(couleur clair)
void SettingsWindow::shape_visu()
{
    if(    ( phi2 != nullptr && !phi2->is_null() )
        && image_shape_uchar != nullptr
        && image_phi_uchar != nullptr   )
    {
        init_width2 = float(width_shape_spin->value())/100.f;
        init_height2 = float(height_shape_spin->value())/100.f;
        center_x2 = float(abscissa_spin->value())/100.f;
        center_y2 = float(ordinate_spin->value())/100.f;

        if( rectangle_radio->isChecked() )
        {
            has_ellipse2 = false;
        }
        else if( ellipse_radio->isChecked() )
        {
            has_ellipse2 = true;
        }

        // efface les listes de la QImage
        if( outsidecolor_combobox->currentIndex() != ComboBoxColorIndex::NO )
        {
            erase_list_to_img( Lout_shape11,
                               image_phi_uchar,
                               image_shape_uchar );
        }

        if( insidecolor_combobox->currentIndex() != ComboBoxColorIndex::NO )
        {
            erase_list_to_img( Lin_shape11,
                               image_phi_uchar,
                               image_shape_uchar );
        }

        Lout_shape11.clear();
        Lin_shape11.clear();
        ofeli_ip::BoundaryBuilder lists_init( phi2->get_width(),
                                              phi2->get_height(),
                                              Lout_shape11,
                                              Lin_shape11 );


        if( has_ellipse2 )
        {
            lists_init.get_ellipse_points( center_x2+0.5f,
                                           center_y2+0.5f,
                                           0.5f*init_width2,
                                           0.5f*init_height2 );
        }
        else
        {
            lists_init.get_rectangle_points( center_x2+0.5f - 0.5f*init_width2,
                                             center_y2+0.5f - 0.5f*init_height2,
                                             center_x2+0.5f + 0.5f*init_width2,
                                             center_y2+0.5f + 0.5f*init_height2 );
        }

        draw_list_to_img( Lout_shape11,
                          color_out_disp,
                          outsidecolor_combobox->currentIndex(),
                          image_shape_uchar, img_width, img_height );


        draw_list_to_img( Lin_shape11,
                          color_in_disp,
                          insidecolor_combobox->currentIndex(),
                          image_shape_uchar, img_width, img_height );

        imageLabel_settings->set_qimage(image_shape);
    }
}

// Surcharge pour avoir une signature identique (memes paramètres) entre signaux et slots de Qt
void SettingsWindow::shape_visu(int)
{
    shape_visu();
}


// Remet phi_init2_clean a zéro, c'est à dire tout correspond à l'extérieur
void SettingsWindow::clean_phi_visu()
{
    if(    ( phi2 != nullptr && !phi2->is_null() )
        && image_phi_uchar != nullptr
        && image_filter_uchar != nullptr
        && image_shape_uchar != nullptr   )
    {
        phi2->memset(ofeli_ip::PhiValue::OUTSIDE_REGION);

        erase_list_to_img1_img2( Lout33,
                                 image_filter_uchar,
                                 image_phi_uchar,
                                 image_shape_uchar );

        erase_list_to_img1_img2( Lin33,
                                 image_filter_uchar,
                                 image_phi_uchar,
                                 image_shape_uchar );

        Lout33.clear();
        Lin33.clear();

        shape_visu();
    }
}

// Soustrait une forme à phi_init2
void SettingsWindow::phi_subtract_shape()
{
    if (    ( displayed_phi_shape != nullptr && !displayed_phi_shape->is_null() )
         && ( phi2 != nullptr && !phi2->is_null() )
         && image_phi_uchar != nullptr
         && image_filter_uchar != nullptr
         && image_shape_uchar != nullptr   )
    {
        do_flood_fill_from_lists(Lout_shape11,Lin_shape11,*displayed_phi_shape);

        int offset;

        bool phi_init2_modif = false;

        for( offset = 0; offset < img_size; offset++ )
        {
            if(     (*displayed_phi_shape)[offset] == ofeli_ip::PhiValue::INSIDE_REGION
                 && (*phi2)[offset] == ofeli_ip::PhiValue::INSIDE_REGION )
            {
                (*phi2)[offset] = ofeli_ip::PhiValue::OUTSIDE_REGION;
                phi_init2_modif = true;
            }
        }

        if( !phi_init2_modif )
        {
            bool has_one = false;
            bool has_minus_one = false;

            for( offset = 0; offset < img_size; offset++ )
            {
                if( (*displayed_phi_shape)[offset] == ofeli_ip::PhiValue::OUTSIDE_REGION )
                {
                    has_one = true;
                }
                else if( (*displayed_phi_shape)[offset] == ofeli_ip::PhiValue::INSIDE_REGION )
                {
                    has_minus_one = true;
                }
            }

            if( has_one && has_minus_one )
            {
                phi_add_shape();
            }
        }
        else
        {
            erase_list_to_img1_img2( Lout33,
                                     image_filter_uchar,
                                     image_phi_uchar,
                                     image_shape_uchar );

            erase_list_to_img1_img2( Lin33,
                                     image_filter_uchar,
                                     image_phi_uchar,
                                     image_shape_uchar );

            find_lists_from_phi(*phi2,Lout33,Lin33);
        }
    }
}

// Ajoute une forme à phi_init2
void SettingsWindow::phi_add_shape()
{
    if (    ( displayed_phi_shape != nullptr && !displayed_phi_shape->is_null() )
         && ( phi2 != nullptr && !phi2->is_null() )
         && image_phi_uchar != nullptr
         && image_filter_uchar != nullptr
         && image_shape_uchar != nullptr   )
    {
        do_flood_fill_from_lists(Lout_shape11,Lin_shape11,*displayed_phi_shape);


        int offset;

        bool phi_init2_modif = false;

        for( offset = 0; offset < img_size; offset++ )
        {
            if( (*displayed_phi_shape)[offset] == ofeli_ip::PhiValue::INSIDE_REGION
                    && (*phi2)[offset] == ofeli_ip::PhiValue::OUTSIDE_REGION )
            {
                (*phi2)[offset] = ofeli_ip::PhiValue::INSIDE_REGION;
                phi_init2_modif = true;
            }
        }

        if( !phi_init2_modif )
        {
            bool has_one = false;
            bool has_minus_one = false;

            for( offset = 0; offset < img_size; offset++ )
            {
                if( (*displayed_phi_shape)[offset] == ofeli_ip::PhiValue::OUTSIDE_REGION )
                {
                    has_one = true;
                }
                else if( (*displayed_phi_shape)[offset] == ofeli_ip::PhiValue::INSIDE_REGION )
                {
                    has_minus_one = true;
                }
            }

            if( has_one && has_minus_one )
            {
                phi_subtract_shape();
            }
        }
        else
        {
            erase_list_to_img1_img2( Lout33,
                                     image_filter_uchar,
                                     image_phi_uchar,
                                     image_shape_uchar );

            erase_list_to_img1_img2( Lin33,
                                     image_filter_uchar,
                                     image_phi_uchar,
                                     image_shape_uchar );

            find_lists_from_phi(*phi2,Lout33,Lin33);
        }
    }
}

// affiche l'image+phi en clair dans la fenêtre de configuration
// dans l'onglet initialisation, affiche l'image+phi en foncé (booléen d'entrée a true) lorsque qu'on clique sur l'image
// ou qu'on appuie sur les bouttons add subtract
void SettingsWindow::phi_visu(bool dark_color)
{
    if(      image_phi_uchar != nullptr
        && image_shape_uchar != nullptr )
    {
        if( outsidecolor_combobox->currentIndex() == ComboBoxColorIndex::SELECTED )
        {
            color_out_disp = selected_out_disp;
        }
        else
        {
            get_color(outsidecolor_combobox->currentIndex(),
                      color_out_disp);
        }

        if( insidecolor_combobox->currentIndex() == ComboBoxColorIndex::SELECTED )
        {
            color_in_disp = selected_in_disp;
        }
        else
        {
            get_color(insidecolor_combobox->currentIndex(),
                      color_in_disp);
        }

        unsigned char color_factor;
        if( dark_color )
        {
            color_factor = 2;
        }
        else
        {
            color_factor = 1;
        }

        if( outsidecolor_combobox->currentIndex() != ComboBoxColorIndex::NO )
        {
            draw_list_to_img1_img2( Lout33,
                                    color_out_disp.divide( color_factor ),
                                    color_out_disp.divide( 2 ),
                                    image_phi_uchar,
                                    image_shape_uchar );
        }

        if( insidecolor_combobox->currentIndex() != ComboBoxColorIndex::NO )
        {
            draw_list_to_img1_img2( Lin33,
                                    color_in_disp.divide( color_factor ),
                                    color_in_disp.divide( 2 ),
                                    image_phi_uchar,
                                    image_shape_uchar );
        }

        if( has_contours_hidden && tabs->currentIndex() == TabIndex::PREPROCESSING )
        {
            // affiche l'ellipse ou le rectangle
            imageLabel_settings->set_qimage(image_filter);
        }
        else
        {
           // affiche l'ellipse ou le rectangle
           imageLabel_settings->set_qimage(image_phi);
        }
    }
}

// Surcharge de phi_visu pour que le signal et le slot de Qt aient la même signature (les memes types de paramètres)
// pour les combo box couleurs
void SettingsWindow::phi_visu(int)
{
    phi_visu(false);
}

// Fonction appelée quand on clique sur le boutton add dans l'onglet initialization
// ou par clic gauche
void SettingsWindow::add_visu()
{
    // ajoute
    phi_add_shape();
    // visualisation de phi en foncé (booléen true)
    phi_visu(true);
}

// Fonction appelée quand on clique sur le boutton subtract dans l'onglet initialization
// ou par clic droit
void SettingsWindow::subtract_visu()
{
    // soustrait
    phi_subtract_shape();
    // visualisation de phi en foncé (booléen true)
    phi_visu(true);
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
    // affichage de l'image avec la nouvelle couleur sélectionnée
    phi_visu(false);

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
    // affichage de l'image avec la nouvelle couleur sélectionnée
    phi_visu(false);
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

// Fonction appelée par tous les widgets de l'onglet Preprocessing
// Calcule l'image prétraitéé
void SettingsWindow::calculate_filtered_copy_visu_buffers()
{
    if(                  img1 != nullptr
        &&           filters2 != nullptr
        && image_filter_uchar != nullptr
        &&    image_phi_uchar != nullptr
        &&  image_shape_uchar != nullptr )
    {
        // récupération de tous les états des widgets de l'onglet preprocessing
        if( page3->isChecked() )
        {
            has_preprocess2 = true;
        }
        else
        {
            has_preprocess2 = false;
        }

        if( gaussian_noise_groupbox->isChecked() )
        {
            has_gaussian_noise2 = true;
        }
        else
        {
            has_gaussian_noise2 = false;
        }
        std_noise2 = float( std_noise_spin->value() );

        if( salt_noise_groupbox->isChecked() )
        {
            has_salt_noise2 = true;
        }
        else
        {
            has_salt_noise2 = false;
        }
        proba_noise2 = float( proba_noise_spin->value() ) / 100.f;

        if( speckle_noise_groupbox->isChecked() )
        {
            has_speckle_noise2 = true;
        }
        else
        {
            has_speckle_noise2 = false;
        }
        std_speckle_noise2 = float( std_speckle_noise_spin->value() );

        if( median_groupbox->isChecked() )
        {
            has_median_filt2 = true;
        }
        else
        {
            has_median_filt2 = false;
        }
        kernel_median_length2 = klength_median_spin->value();
        if( complex_radio2->isChecked() )
        {
            has_O1_algo2 = true;
        }
        else
        {
            has_O1_algo2 = false;
        }

        if( mean_groupbox->isChecked() )
        {
            has_mean_filt2 = true;
        }
        else
        {
            has_mean_filt2 = false;
        }
        kernel_mean_length2 = klength_mean_spin->value();

        if( gaussian_groupbox->isChecked() )
        {
            has_gaussian_filt2 = true;
        }
        else
        {
            has_gaussian_filt2 = false;
        }
        kernel_gaussian_length2 = klength_gaussian_spin->value();
        sigma2 = float( std_filter_spin->value() );


        if( aniso_groupbox->isChecked() )
        {
            has_aniso_diff2 = true;
        }
        else
        {
            has_aniso_diff2 = false;
        }

        max_itera2 = iteration_filter_spin->value();
        lambda2 = float( lambda_spin->value() );
        kappa2 = float( kappa_spin->value() );
        if( aniso1_radio->isChecked() )
        {
            aniso_option2 = ofeli_ip::AnisoDiff::FUNCTION1;
        }
        else if( aniso2_radio->isChecked() )
        {
            aniso_option2 = ofeli_ip::AnisoDiff::FUNCTION2;
        }

        if( open_groupbox->isChecked() )
        {
            has_open_filt2 = true;
        }
        else
        {
            has_open_filt2 = false;
        }
        kernel_open_length2 = klength_open_spin->value();

        if( close_groupbox->isChecked() )
        {
            has_close_filt2 = true;
        }
        else
        {
            has_close_filt2 = false;
        }
        kernel_close_length2 = klength_close_spin->value();

        if( tophat_groupbox->isChecked() )
        {
            has_top_hat_filt2 = true;
        }
        else
        {
            has_top_hat_filt2 = false;
        }
        if( whitetophat_radio->isChecked() )
        {
            is_white_top_hat2 = true;
        }
        else
        {
            is_white_top_hat2 = false;
        }
        kernel_tophat_length2 = klength_tophat_spin->value();

        if( complex2_morpho_radio->isChecked() )
        {
            has_O1_morpho2 = true;
        }
        else
        {
            has_O1_morpho2 = false;
        }

        if( chanvese_radio->isChecked() )
        {
            speed = SpeedModel::REGION_BASED;
        }
        else if( geodesic_radio->isChecked() )
        {
            speed = SpeedModel::EDGE_BASED;
        }
        kernel_gradient_length2 = klength_gradient_spin->value();

        alpha2 = alpha_spin->value();
        beta2 = beta_spin->value();
        gamma2 = gamma_spin->value();

        if( histo_checkbox->isChecked() )
        {
            has_histo_normaliz2 = true;
        }
        else
        {
            has_histo_normaliz2 = false;
        }


        if( has_open_filt2 || has_close_filt2 || has_top_hat_filt2 )
        {
            algo_groupbox->setEnabled(true);
        }
        else
        {
            algo_groupbox->setEnabled(false);
        }

        float elapsed_time = calculate_filtered_image();

        time_filt->setText(tr("time = ")+QString::number(elapsed_time)+" s");



        unsigned char I;

        if( speed == SpeedModel::REGION_BASED )
        {
            if( is_rgb1 )
            {
                std::memcpy(image_filter_uchar,img2_filtered,4*img_size);
                std::memcpy(image_phi_uchar,img2_filtered,4*img_size);
                std::memcpy(image_shape_uchar,img2_filtered,4*img_size);
            }
            else
            {
                for( int offset = 0; offset < img_size; offset++ )
                {
                    I = img2_filtered[offset];

                    image_filter_uchar[4*offset+2] = I;
                    image_filter_uchar[4*offset+1] = I;
                    image_filter_uchar[4*offset  ] = I;

                    image_phi_uchar[4*offset+2] = I;
                    image_phi_uchar[4*offset+1] = I;
                    image_phi_uchar[4*offset  ] = I;

                    image_shape_uchar[4*offset+2] = I;
                    image_shape_uchar[4*offset+1] = I;
                    image_shape_uchar[4*offset  ] = I;
                }
            }
        }
        else if( speed == SpeedModel::EDGE_BASED )
        {
            if( has_histo_normaliz2 )
            {
                unsigned char max = 0;
                unsigned char min = 255;

                for( int offset = 0; offset < img_size; offset++ )
                {
                    if( img2_filtered[offset] > max )
                    {
                        max = img2_filtered[offset];
                    }
                    if( img2_filtered[offset] < min )
                    {
                        min = img2_filtered[offset];
                    }
                }

                for( int offset = 0; offset < img_size; offset++ )
                {
                    I = (unsigned char)(255.f*float(img2_filtered[offset]-min)/float(max-min));

                    image_filter_uchar[4*offset+2] = I;
                    image_filter_uchar[4*offset+1] = I;
                    image_filter_uchar[4*offset  ] = I;

                    image_phi_uchar[4*offset+2] = I;
                    image_phi_uchar[4*offset+1] = I;
                    image_phi_uchar[4*offset  ] = I;

                    image_shape_uchar[4*offset+2] = I;
                    image_shape_uchar[4*offset+1] = I;
                    image_shape_uchar[4*offset  ] = I;
                }
            }
            else
            {
                for( int offset = 0; offset < img_size; offset++ )
                {
                    I = img2_filtered[offset];

                    image_filter_uchar[4*offset+2] = I;
                    image_filter_uchar[4*offset+1] = I;
                    image_filter_uchar[4*offset  ] = I;

                    image_phi_uchar[4*offset+2] = I;
                    image_phi_uchar[4*offset+1] = I;
                    image_phi_uchar[4*offset  ] = I;

                    image_shape_uchar[4*offset+2] = I;
                    image_shape_uchar[4*offset+1] = I;
                    image_shape_uchar[4*offset  ] = I;
                }
            }
        }
    }
}

void SettingsWindow::show_phi_with_filtered_image()
{
    calculate_filtered_copy_visu_buffers();
    phi_visu(false);
}

// Fonction appelée lorsque on change de taille d'image dans l'onglet Display
void SettingsWindow::change_display_size()
{
    if( img1 != nullptr )
    {
        phi_visu(false);
    }
}

// Fonction appelée à chaque changement d'onglet
void SettingsWindow::tab_visu(int value)
{
    if( img1 != nullptr )
    {
        if( value == TabIndex::INITIALIZATION )
        {
            imageLabel_settings->set_doWheelEvent(false);
            // visualisation de la forme
            phi_visu(true);
            shape_visu();
        }
        else
        {
            imageLabel_settings->set_doWheelEvent(true);
            // visualisation de phi
            phi_visu(false);
        }
    }
}

// enlever des trucs de mainwindow
// Filtre des événements pour avoir le tracking au niveau du widget image de la fenêtre principale et de la fenêtre de parametres et pour ne pas avoir le tracking/la position au niveau de l'ensemble de chaque fenêtre
bool SettingsWindow::eventFilter(QObject* object, QEvent* event)
{
    if( object == imageLabel_settings )
    {
        if( event->type() == QEvent::DragEnter )
        {
            QDragEnterEvent* drag = static_cast<QDragEnterEvent*>(event);
            drag_enter_event_phi(drag);
        }
        else if( event->type() == QEvent::DragMove )
        {
            QDragMoveEvent* drag = static_cast<QDragMoveEvent*>(event);
            drag_move_event_phi(drag);
        }
        else if( event->type() == QEvent::Drop )
        {
            QDropEvent* drag = static_cast<QDropEvent*>(event);
            drop_event_phi(drag);
        }
        else if( event->type() == QEvent::DragLeave )
        {
            QDragLeaveEvent* drag = static_cast<QDragLeaveEvent*>(event);
            drag_leave_event_phi(drag);
        }
        else if( event->type() == QEvent::MouseMove )
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouse_move_event_settings(mouseEvent);
        }
        else if( event->type() == QEvent::MouseButtonPress )
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouse_press_event_settings(mouseEvent);
        }
    }
    else if( object == scale_spin || object == scale_slider )
    {
        if( event->type() == QEvent::MouseButtonPress )
        {
            if( img1 != nullptr )
            {
                imageLabel_settings->set_has_text(false);
                imageLabel_settings->setBackgroundRole(QPalette::Dark);
            }
            positionX = img_width/2;
            positionY = img_height/2;
        }
    }

    return false;
}

// Evénement de déplacement de la souris dans le widget image de la fenêtre de paramètres
void SettingsWindow::mouse_move_event_settings(QMouseEvent* event)
{
    // si l'image est chargée et si on est dans l'onglet initialisation
    if( img1 != nullptr )
    {
        // position de la souris dans l'image
        positionX = int(float(img_width)*float(   (  (event->pos()).x() -imageLabel_settings->get_xoffset()   ) /float(imageLabel_settings->getPixWidth())));
        positionY = int(float(img_height)*float(   (  (event->pos()).y() -imageLabel_settings->get_yoffset()   ) /float(imageLabel_settings->getPixHeight())));

        if( tabs->currentIndex() == TabIndex::INITIALIZATION )
        {
            // on en deduit les valeurs relatives des sliders en %

            float a = (float(positionX)-float(img_width/2))/float(img_width);
            float b = (float(positionY)-float(img_height/2))/float(img_height);

            abscissa_spin->setValue(int(a*100.f));
            ordinate_spin->setValue(int(b*100.f));
        }
    }

}

// Evénement clic souris dans le widget image de la fenêtre de paramètres
void SettingsWindow::mouse_press_event_settings(QMouseEvent* event)
{
    if( (img1 != nullptr) && tabs->currentIndex() == TabIndex::INITIALIZATION )
    {
        if( event->button() == Qt::LeftButton )
        {
            add_visu();
        }
        else if( event->button() == Qt::RightButton )
        {
            subtract_visu();
        }
        /*
        else if( event->button() == Qt::MidButton )
        {
            if( !rectangle_radio->isChecked() )
            {
                rectangle_radio->setChecked(true);
            }
            else
            {
                ellipse_radio->setChecked(true);
            }
            shape_visu();
        }*/
    }

    if( img1 != nullptr && tabs->currentIndex() == TabIndex::PREPROCESSING )
    {
        if( event->button() == Qt::RightButton )
        {
            if( has_contours_hidden )
            {
                has_contours_hidden = false;
            }
            else
            {
                has_contours_hidden = true;
            }
            phi_visu(false);
        }
        else if( event->button() == Qt::LeftButton )
        {
            has_contours_hidden = true;
            if( has_show_img1 )
            {
                has_show_img1 = false;
                img1_visu();
            }
            else
            {
                has_show_img1 = true;
                phi_visu(false);
            }
        }
    }
 }

// Fonction pour afficher l'image de départ par clic gauche dans l'onglet preprocessing
void SettingsWindow::img1_visu()
{
    if( img1 != nullptr )
    {
        imageLabel_settings->set_qimage(img);
    }
}

void SettingsWindow::drag_enter_event_phi(QDragEnterEvent* event)
{
    if( tabs->currentIndex() != TabIndex::INITIALIZATION )
    {
        tabs->setCurrentIndex(TabIndex::INITIALIZATION);
    }
    QString text(tr("<drop ϕ(t=0)>"));
    imageLabel_settings->set_text(text);
    imageLabel_settings->setBackgroundRole(QPalette::Highlight);
    imageLabel_settings->set_has_text(true);

    event->acceptProposedAction();
    emit changed(event->mimeData());
}

void SettingsWindow::drag_move_event_phi(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void SettingsWindow::drop_event_phi(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if( mimeData->hasUrls() )
    {
        QList<QUrl> urlList = mimeData->urls();
        fileName_phi = urlList.first().toLocalFile();
    }
    imageLabel_settings->setBackgroundRole(QPalette::Dark);
    open_phi();
    event->acceptProposedAction();
}

void SettingsWindow::drag_leave_event_phi(QDragLeaveEvent* event)
{
    QString text(tr("<drag ϕ(t=0)>"));
    imageLabel_settings->set_text(text);
    imageLabel_settings->setBackgroundRole(QPalette::Dark);
    imageLabel_settings->set_has_text(true);

    emit changed();
    event->accept();
}

void SettingsWindow::openFilenamePhi()
{
    fileName_phi = QFileDialog::getOpenFileName(this,
                                            tr("Open File"),
                                            last_directory_used,
                                            tr("Image Files (%1)").arg(nameFilters.join(" ")));

    open_phi();
}

void SettingsWindow::open_phi()
{
    if( img1 != nullptr )
    {
        if( !fileName_phi.isEmpty() )
        {
            QFileInfo fi(fileName_phi);
            last_directory_used = fi.absolutePath();

            QImage img_phi1(fileName_phi);

            if( img_phi1.isNull() )
            {
                QMessageBox::information(this, tr("Opening error - Ofeli"),
                                         tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName_phi)));
                return;
            }

            img_phi1 = img_phi1.scaled(img_width, img_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);

            int histogram[256];
            for( unsigned int I = 0; I <= 255u; I++ )
            {
                histogram[I] = 0;
            }

            if( img_phi1.format() == QImage::Format_Indexed8 )
            {

                for( int y = 0; y < img_height; y++ )
                {
                    for( int x = 0; x < img_width; x++ )
                    {
                        histogram[ *( img_phi1.scanLine(y)+x ) ]++;
                    }
                }

                const unsigned char threshold = otsu_method(histogram,img_size);

                for( int y = 0; y < img_height; y++ )
                {
                    for( int x = 0; x < img_width; x++ )
                    {
                        if( *( img_phi1.scanLine(y)+x ) <= threshold )
                        {
                            (*phi2)(x,y) = ofeli_ip::PhiValue::OUTSIDE_REGION;
                        }
                        else
                        {
                            (*phi2)(x,y) = ofeli_ip::PhiValue::INSIDE_REGION;
                        }
                    }
                }

            }
            else
            {
                QRgb pix;

                for( int y = 0; y < img_height; y++ )
                {
                    for( int x = 0; x < img_width; x++ )
                    {
                        pix = img_phi1.pixel(x,y);

                        histogram[ (unsigned char) ( 0.2989f*(float)(qRed(pix))
                                                     + 0.5870f*(float)(qGreen(pix))
                                                     + 0.1140f*(float)(qBlue(pix)) ) ]++;
                    }
                }

                const unsigned char threshold = otsu_method(histogram,img_size);

                for( int y = 0; y < img_height; y++ )
                {
                    for( int x = 0; x < img_width; x++ )
                    {
                        pix = img_phi1.pixel(x,y);

                        if( (unsigned char) ( 0.2989f*(float)(qRed(pix))
                                              + 0.5870f*(float)(qGreen(pix))
                                              + 0.1140f*(float)(qBlue(pix)) ) <= threshold )
                        {
                            (*phi2)(x,y) = ofeli_ip::PhiValue::OUTSIDE_REGION;
                        }
                        else
                        {
                            (*phi2)(x,y) = ofeli_ip::PhiValue::INSIDE_REGION;
                        }
                    }
                }
            }

            erase_list_to_img1_img2( Lout33,
                                     image_filter_uchar,
                                     image_phi_uchar,
                                     image_shape_uchar );

            erase_list_to_img1_img2( Lin33,
                                     image_filter_uchar,
                                     image_phi_uchar,
                                     image_shape_uchar );

            Lout33.clear();
            Lin33.clear();

            find_lists_from_phi(*phi2,Lout33,Lin33);
            tab_visu( tabs->currentIndex() );
        }
    }
}

void SettingsWindow::save_phi_without_given_extension(const QString& img_str,
                                                      const QString& selected_filter,
                                                      const QString& fileName_save,
                                                      const QImage& img_phi_save)
{
    if( selected_filter.contains(img_str) )
    {
        if( img_phi_save.save(fileName_save + ".png") )
        {
            QMessageBox::information(this, tr("Opening error - Ofeli"),
                                     tr("Cannot save %1.").arg(QDir::toNativeSeparators(fileName_save)));
        }
        else
        {
            // error
        }
    }
    else
    {
        int first_point = selected_filter.indexOf(".");

        if( first_point != -1 )
        {
            QString selected_extension = selected_filter.right(first_point-1);

            if( !img_phi_save.save(fileName_save + selected_extension) )
            {
                QMessageBox::information(this, tr("Opening error - Ofeli"),
                                         tr("Cannot save %1.").arg(QDir::toNativeSeparators(fileName_save)));
            }
        }
        else
        {
            std::cerr << "Error." << std::endl;
        }
    }
}

void SettingsWindow::save_phi()
{
    if( phi2 != nullptr && !phi2->is_null() )
    {
        QImage img_phi_save = QImage(img_width,img_height,QImage::Format_Indexed8);

        for( int y = 0; y < img_height; y++ )
        {
            for( int x = 0; x < img_width; x++ )
            {
                if( (*phi2)(x,y) == ofeli_ip::PhiValue::OUTSIDE_REGION )
                {
                    *(img_phi_save.scanLine(y)+x) = 0;
                }
                else if( (*phi2)(x,y) == ofeli_ip::PhiValue::INSIDE_REGION )
                {
                    *(img_phi_save.scanLine(y)+x) = 255;
                }
                else
                {
                    std::cerr << "Error." << std::endl;
                }
            }
        }

        int y;

        for( const auto& point : Lout33 )
        {
            y = point.get_offset()/img_width;
            *(img_phi_save.scanLine(y)+point.get_x()) = 85;
        }

        for( const auto& point : Lin33 )
        {
            y = point.get_offset()/img_width;
            *(img_phi_save.scanLine(y)+point.get_x()) = 170;
        }




        QVector<QRgb> table(256);
        for( int I = 0; I < 256; I++ )
        {
            table[I] = qRgb(I,I,I);
        }
        img_phi_save.setColorTable(table);

        QString img_str(tr("Images"));
        QString filters = img_str + " (*.bmp *.jpg *.jpeg *.png *.ppm *.xbm *.xpm);;BMP (*.bmp);;JPG JPEG (*.jpg *.jpeg);;PNG (*.png);;PPM (*.ppm);;XBM (*.xbm);;XPM (*.xpm)";
        QString selected_filter;
        QString fileName_save = QFileDialog::getSaveFileName(this,
                                                             tr("Save ϕ(t=0)"),
                                                             last_directory_used + QString(tr("/initial_phi_data")),
                                                             filters,
                                                             &selected_filter);


        int last_point = fileName_save.lastIndexOf(".");

        if( last_point != -1 )
        {
            QString extension = fileName_save.right(last_point-1);

            if( filters.contains(extension) )
            {
                if( !img_phi_save.save(fileName_save) )
                {
                    QMessageBox::information(this, tr("Opening error - Ofeli"),
                                             tr("Cannot save %1.").arg(QDir::toNativeSeparators(fileName_save)));
                }
            }
            else
            {
                save_phi_without_given_extension(img_str,
                                                 selected_filter,
                                                 fileName_save,
                                                 img_phi_save);
            }
        }
        else
        {
            save_phi_without_given_extension(img_str,
                                             selected_filter,
                                             fileName_save,
                                             img_phi_save);
        }
    }
}

void SettingsWindow::adjustVerticalScroll_settings(int min, int max)
{
    if( img_height >= 1 )
    {
        scrollArea_settings->verticalScrollBar()->setValue( (max-min)*positionY/img_height );
    }
}

void SettingsWindow::adjustHorizontalScroll_settings(int min, int max)
{
    if( img_width >= 1 )
    {
        scrollArea_settings->horizontalScrollBar()->setValue( (max-min)*positionX/img_width );
    }
}

unsigned char SettingsWindow::otsu_method(const int histogram[], unsigned int img_size)
{
    unsigned int sum = 0;
    for( unsigned int I = 0; I <= 255; I++ )
    {
        sum += I*histogram[I];
    }

    unsigned int weight1, weight2, sum1;
    float mean1, mean2, var_t, var_max;

    unsigned char threshold = 127; // value returned in the case of an totally homogeneous image

    weight1 = 0;
    sum1 = 0;
    var_max = -1.f;

    // 256 values ==> 255 thresholds t evaluated
    // class1 <= t and class2 > t
    for( unsigned char t = 0; t < 255; t++ )
    {
        weight1 += histogram[t];
        if( weight1 == 0 )
        {
            continue;
        }

        weight2 = img_size-weight1;
        if( weight2 == 0 )
        {
            break;
        }

        sum1 += t*histogram[t];

        mean1 = float(sum1/weight1);
        mean2 = float( (sum-sum1)/weight2 ); // sum2 = sum-sum1

        var_t = float(weight1)*float(weight2)*(mean1-mean2)*(mean1-mean2);

        if( var_t > var_max )
        {
            var_max = var_t;
            threshold = t;
        }
    }

    return threshold;
}

void SettingsWindow::wheel_zoom(int val, ScrollAreaWidget* obj)
{
    if( obj == scrollArea_settings && img1 != nullptr )
    {
        imageLabel_settings->set_has_text(false);
        imageLabel_settings->setBackgroundRole(QPalette::Dark);

        if( tabs->currentIndex() == TabIndex::INITIALIZATION )
        {
            // nombre de degré et de pas de la molette
            int numDegrees = val / 8;
            int numSteps = numDegrees / 15;

            // récupération des valeurs des sliders concernant la taille de la forme
            int width_ratio = width_shape_spin->value();
            int height_ratio = height_shape_spin->value();

            // correpondance par rapport à la taille réelle de la forme
            float width = float(img_width)*float(width_ratio)/100.f;
            float height = float(img_height)*float(height_ratio)/100.f;

            // augmentation ou diminution de 15% à chaque pas de molette
            width += 0.15f*float(numSteps)*width;
            height += 0.15f*float(numSteps)*height;

            width_ratio = int( 100.f*width/float(img_width) );
            height_ratio = int( 100.f*height/float(img_height) );

            if( (width_ratio > 3) && (height_ratio > 3) && (width_ratio < 150) && (height_ratio < 150) )
            {
                width_shape_spin->setValue(width_ratio);
                height_shape_spin->setValue(height_ratio);
            }
        }
        else
        {
            float value = 0.002f*float( val ) + imageLabel_settings->get_zoomFactor();

            if( value < 32.f/float( imageLabel_settings->get_qimage().width() ) )
            {
                value = 32.f/float( imageLabel_settings->get_qimage().width() );
            }

            scale_spin->setValue( int(100.f*value) );
        }
    }
}

void SettingsWindow::do_flood_fill_from_lists(const std::vector<ofeli_ip::ContourPoint>& Lout,
                                              const std::vector<ofeli_ip::ContourPoint>& Lin,
                                              ofeli_ip::Matrix<signed char>& phi)
{
    phi.memset(ofeli_ip::PhiValue::OUTSIDE_REGION);

    for( const auto& point : Lout )
    {
        phi[ point.get_offset() ] = ofeli_ip::PhiValue::EXTERIOR_BOUNDARY;
    }

    for( const auto& point : Lin )
    {
        do_flood_fill( phi,
                       point.get_offset(),
                       ofeli_ip::PhiValue::OUTSIDE_REGION,
                       ofeli_ip::PhiValue::INSIDE_REGION );
    }

    for( const auto& point : Lout )
    {
        phi[ point.get_offset() ] = ofeli_ip::PhiValue::OUTSIDE_REGION;
    }
}

void SettingsWindow::find_lists_from_phi(const ofeli_ip::Matrix<signed char>& phi,
                                         std::vector<ofeli_ip::ContourPoint>& Lout,
                                         std::vector<ofeli_ip::ContourPoint>& Lin)
{
    Lout.clear();
    Lin.clear();
// vite fait a optimiser ensuite
    const int phi_size = phi.get_width()*phi.get_height();
    int x, y;

    for( int offset = 0; offset < phi_size; offset++ )
    {
        phi.get_position(offset, x, y);

        if( phi[offset] < ofeli_ip::PhiValue::ZERO_LEVEL_SET )
        {
            if( !find_redundant_list_point(phi, offset) )
            {
                Lin.emplace_back(offset, x);
            }
        }
        else
        {
            if( !find_redundant_list_point(phi, offset) )
            {
                Lout.emplace_back(offset, x);
            }
        }
    }
}

bool SettingsWindow::find_redundant_list_point(const ofeli_ip::Matrix<signed char>& phi,
                                               int offset) const
{
    int x, y;
    phi.get_position(offset,x,y); // x and y passed by reference

    if( x > 0 )
    {
        if( phi(x-1,y)*phi[offset] < 0 )
        {
            return false;
        }
    }
    if( x < phi.get_width()-1 )
    {
        if( phi(x+1,y)*phi[offset] < 0 )
        {
            return false;
        }
    }

    if( y > 0 )
    {
        if( phi(x,y-1)*phi[offset] < 0 )
        {
            return false;
        }

#ifdef ALGO_8_CONNEXITY
        if( x > 0 )
        {
            if( phi(x-1,y-1)*phi[offset] < 0 )
            {
                return false;
            }
        }
        if( x < phi.get_width()-1 )
        {
            if( phi(x+1,y-1)*phi[offset] < 0 )
            {
                return false;
            }
        }
#endif

    }

    if( y < phi.get_height()-1 )
    {
        if( phi(x,y+1)*phi[offset] < 0 )
        {
            return false;
        }

#ifdef ALGO_8_CONNEXITY
        if( x > 0 )
        {
            if( phi(x-1,y+1)*phi[offset] < 0 )
            {
                return false;
            }
        }
        if( x < phi.get_width()-1 )
        {
            if( phi(x+1,y+1)*phi[offset] < 0 )
            {
                return false;
            }
        }
#endif

    }

    return true;
}

void SettingsWindow::do_flood_fill(ofeli_ip::Matrix<signed char>& phi, int offset_seed,
                                   ofeli_ip::PhiValue target_value, ofeli_ip::PhiValue replacement_value)
{
    if( target_value != replacement_value &&
        offset_seed < phi.get_width()*phi.get_height()-1 )
    {
        std::stack<int> offset_seeds;
        // top seed coordinates (x_ts,y_ts) and x for scan the row
        int x_ts, y_ts, x;
        bool span_up, span_down;

        offset_seeds.push(offset_seed);

        while( !offset_seeds.empty() )
        {
            // unstack the top seed
            phi.get_position(offset_seeds.top(),
                             x_ts,y_ts); // x_ts and y_ts passed by reference
            offset_seeds.pop();

            // x initialization at the left-most point of the seed
            x = x_ts;
            while( x > 0 &&
                   phi(x-1,y_ts) == target_value )
            {
                x--;
            }

            span_up = false;
            span_down = false;

            // pixels are treated row-wise
            while( x < phi.get_width() &&
                   phi(x,y_ts) == target_value )
            {
                phi(x,y_ts) = replacement_value;

                if( !span_up &&
                    y_ts > 0 &&
                    phi(x,y_ts-1) == target_value )
                {
                    offset_seeds.push( phi.get_offset(x,y_ts-1) );
                    span_up = true;
                }
                else if( span_up &&
                           y_ts > 0 &&
                           phi(x,y_ts-1) != target_value )
                {
                    span_up = false;
                }

                if( !span_down &&
                    y_ts < phi.get_height()-1 &&
                    phi(x,y_ts+1) == target_value )
                {
                    offset_seeds.push( phi.get_offset(x,y_ts+1) );
                    span_down = true;
                }
                else if( span_down &&
                           y_ts < phi.get_height()-1
                           && phi(x,y_ts+1) != target_value )
                {
                    span_down = false;
                }

                x++;
            }
        }
    }
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue( "Settings/Window/geometry", saveGeometry() );

    QDialog::closeEvent(event);
}

}
