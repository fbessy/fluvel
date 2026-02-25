// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "algo_settings_widget.hpp"
#include "application_settings.hpp"

#include <cassert>
#include <utility>

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>

namespace ofeli_app
{

AlgoSettingsWidget::AlgoSettingsWidget(QWidget* parent, Session session)
    : QWidget(parent)
    , session_(session)
    , config_(config(session))
{
    connectivity_cb_ = new QComboBox;

    connectivity_cb_->addItem("4-connected (Von Neumann)",
                              QVariant::fromValue(ofeli_ip::Connectivity::Four));

    connectivity_cb_->addItem("8-connected (Moore)",
                              QVariant::fromValue(ofeli_ip::Connectivity::Eight));

    QFormLayout* connect_layout = new QFormLayout;
    connect_layout->addRow("Connectivity :", connectivity_cb_);

    QGroupBox* externalspeed_groupbox = new QGroupBox(tr("Cycle 1 – Data-driven evolution"));

    Na_spin_ = new QSpinBox;
    Na_spin_->setSingleStep(1);
    Na_spin_->setMinimum(1);
    Na_spin_->setMaximum(999);
    Na_spin_->setSuffix(tr(" iterations"));
    Na_spin_->setToolTip(tr("Number of iterations of the data-driven evolution (Cycle 1)."));
    QFormLayout* Na_layout = new QFormLayout;
    Na_layout->addRow("Na =", Na_spin_);

    lambda_out_spin_ = new QSpinBox;
    lambda_out_spin_->setSingleStep(1);
    lambda_out_spin_->setMinimum(1);
    lambda_out_spin_->setMaximum(100000);
    lambda_out_spin_->setToolTip(tr("weight of the outside homogeneity criterion"));

    lambda_in_spin_ = new QSpinBox;
    lambda_in_spin_->setSingleStep(1);
    lambda_in_spin_->setMinimum(1);
    lambda_in_spin_->setMaximum(100000);
    lambda_in_spin_->setToolTip(tr("weight of the inside homogeneity criterion"));

    QFormLayout* lambda_layout = new QFormLayout;

    auto& config = AppSettings::instance().imgConfig.display;

    QColor RGBout_list(get_QRgb(config.l_out_color));
    QColor RGBin_list(get_QRgb(config.l_in_color));

    lambda_layout->addRow("<font color=" + RGBout_list.name() + ">" + "λout" +
                              "<font color=black>" + " =",
                          lambda_out_spin_);
    lambda_layout->addRow("<font color=" + RGBin_list.name() + ">" + "λin" + "<font color=black>" +
                              " =",
                          lambda_in_spin_);

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addLayout(lambda_layout);

    QHBoxLayout* speed_layout = new QHBoxLayout;
    speed_layout->addLayout(chanvese_layout);

    color_weights_groupbox_ = new QGroupBox(tr("Color space"));
    color_weights_groupbox_->setFlat(true);

    color_space_cb_ = new QComboBox;
    color_space_cb_->addItem("RGB", QVariant::fromValue(ofeli_ip::ColorSpaceOption::RGB));

    color_space_cb_->addItem("YUV", QVariant::fromValue(ofeli_ip::ColorSpaceOption::YUV));

    color_space_cb_->addItem("L*a*b* (CIELAB)",
                             QVariant::fromValue(ofeli_ip::ColorSpaceOption::Lab));

    color_space_cb_->addItem("L*u*v* (CIELUV)",
                             QVariant::fromValue(ofeli_ip::ColorSpaceOption::Luv));

    alpha_spin_ = new QSpinBox;
    alpha_spin_->setSingleStep(1);
    alpha_spin_->setMinimum(1);
    alpha_spin_->setMaximum(100000);

    beta_spin_ = new QSpinBox;
    beta_spin_->setSingleStep(1);
    beta_spin_->setMinimum(1);
    beta_spin_->setMaximum(100000);

    gamma_spin_ = new QSpinBox;
    gamma_spin_->setSingleStep(1);
    gamma_spin_->setMinimum(1);
    gamma_spin_->setMaximum(100000);

    QFormLayout* color_weights_layout = new QFormLayout;
    color_weights_layout->addRow(tr("1st component weight ="), alpha_spin_);
    color_weights_layout->addRow(tr("2nd component weight ="), beta_spin_);
    color_weights_layout->addRow(tr("3rd component weight ="), gamma_spin_);

    QVBoxLayout* vcolor_layout = new QVBoxLayout;
    vcolor_layout->addWidget(color_space_cb_);
    vcolor_layout->addLayout(color_weights_layout);

    color_weights_groupbox_->setLayout(vcolor_layout);

    QVBoxLayout* externalspeed_layout = new QVBoxLayout;

    externalspeed_layout->addLayout(Na_layout);
    externalspeed_layout->addLayout(speed_layout);
    externalspeed_layout->addWidget(color_weights_groupbox_);
    externalspeed_groupbox->setLayout(externalspeed_layout);

    ////////////////////////////////////////////

    internalspeed_groupbox_ = new QGroupBox(tr("Cycle 2 - Internal smoothing"));
    internalspeed_groupbox_->setCheckable(true);
    internalspeed_groupbox_->setChecked(true);

    Ns_spin_ = new QSpinBox;
    Ns_spin_->setSingleStep(1);
    Ns_spin_->setMinimum(1);
    Ns_spin_->setMaximum(999);
    Ns_spin_->setSuffix(tr(" iterations"));
    Ns_spin_->setToolTip(tr("Number of internal smoothing iterations (Cycle 2)."));

    disk_radius_spin_ = new QSpinBox;
    disk_radius_spin_->setSingleStep(1);
    disk_radius_spin_->setMinimum(1);
    disk_radius_spin_->setMaximum(999);
    disk_radius_spin_->setToolTip(tr("Radius of the disk-shaped neighborhood used for the "
                                     "majority vote during internal smoothing."));

    QFormLayout* internalspeed_layout = new QFormLayout;
    internalspeed_layout->addRow("Ns =", Ns_spin_);
    internalspeed_layout->addRow("R =", disk_radius_spin_);

    internalspeed_groupbox_->setLayout(internalspeed_layout);

    ////////////////////////////////////////////

    QVBoxLayout* algorithm_layout = new QVBoxLayout;
    algorithm_layout->setContentsMargins(0, 0, 0, 0);
    algorithm_layout->addLayout(connect_layout);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(externalspeed_groupbox);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(internalspeed_groupbox_);
    algorithm_layout->addStretch(1);

    setLayout(algorithm_layout);

    // to initialize ui state in function of the configuration
    reject();
}

void AlgoSettingsWidget::accept()
{
    config_.connectivity = connectivity_cb_->currentData().value<ofeli_ip::Connectivity>();

    config_.acConfig.Na = Na_spin_->value();
    config_.acConfig.Ns = Ns_spin_->value();
    config_.acConfig.is_cycle2 = internalspeed_groupbox_->isChecked();
    config_.acConfig.disk_radius = disk_radius_spin_->value();

    config_.regionAcConfig.lambda_out = lambda_out_spin_->value();
    config_.regionAcConfig.lambda_in = lambda_in_spin_->value();

    config_.regionAcConfig.color_space =
        color_space_cb_->currentData().value<ofeli_ip::ColorSpaceOption>();

    config_.regionAcConfig.weights.c1 = alpha_spin_->value();
    config_.regionAcConfig.weights.c2 = beta_spin_->value();
    config_.regionAcConfig.weights.c3 = gamma_spin_->value();
}

void AlgoSettingsWidget::reject()
{
    int index = connectivity_cb_->findData(QVariant::fromValue(config_.connectivity));
    if (index >= 0)
        connectivity_cb_->setCurrentIndex(index);

    Na_spin_->setValue(config_.acConfig.Na);
    Ns_spin_->setValue(config_.acConfig.Ns);
    internalspeed_groupbox_->setChecked(config_.acConfig.is_cycle2);
    disk_radius_spin_->setValue(config_.acConfig.disk_radius);

    lambda_out_spin_->setValue(config_.regionAcConfig.lambda_out);
    lambda_in_spin_->setValue(config_.regionAcConfig.lambda_in);

    index = color_space_cb_->findData(QVariant::fromValue(config_.regionAcConfig.color_space));
    if (index >= 0)
        color_space_cb_->setCurrentIndex(index);

    alpha_spin_->setValue(config_.regionAcConfig.weights.c1);
    beta_spin_->setValue(config_.regionAcConfig.weights.c2);
    gamma_spin_->setValue(config_.regionAcConfig.weights.c3);
}

AlgoConfig& AlgoSettingsWidget::config(Session session)
{
    switch (session)
    {
        case Session::Image:
            return AppSettings::instance().imgConfig.compute.algo;

        case Session::Camera:
            return AppSettings::instance().camConfig.compute.algo;
    }

    std::unreachable();
}

} // namespace ofeli_app
