// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "algo_settings_widget.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>

#include <cassert>

namespace fluvel_app
{

AlgoSettingsWidget::AlgoSettingsWidget(ActiveContourConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    connectivity_cb_ = new QComboBox;

    connectivity_cb_->addItem("4-connected (Von Neumann)",
                              QVariant::fromValue(fluvel_ip::Connectivity::Four));

    connectivity_cb_->addItem("8-connected (Moore)",
                              QVariant::fromValue(fluvel_ip::Connectivity::Eight));

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

    lambda_layout->addRow("λout =", lambda_out_spin_);
    lambda_layout->addRow("λin =", lambda_in_spin_);

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addLayout(lambda_layout);

    QHBoxLayout* speed_layout = new QHBoxLayout;
    speed_layout->addLayout(chanvese_layout);

    color_weights_groupbox_ = new QGroupBox(tr("Color space"));
    color_weights_groupbox_->setFlat(true);

    color_space_cb_ = new QComboBox;
    color_space_cb_->addItem("RGB", QVariant::fromValue(fluvel_ip::ColorSpaceOption::RGB));

    color_space_cb_->addItem("YUV", QVariant::fromValue(fluvel_ip::ColorSpaceOption::YUV));

    color_space_cb_->addItem("L*a*b* (CIELAB)",
                             QVariant::fromValue(fluvel_ip::ColorSpaceOption::Lab));

    color_space_cb_->addItem("L*u*v* (CIELUV)",
                             QVariant::fromValue(fluvel_ip::ColorSpaceOption::Luv));

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

    connect(connectivity_cb_, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                auto connectivity =
                    connectivity_cb_->itemData(index).value<fluvel_ip::Connectivity>();

                emit connectivityChanged(connectivity);
            });
}

void AlgoSettingsWidget::accept()
{
    config_.connectivity = connectivity_cb_->currentData().value<fluvel_ip::Connectivity>();

    config_.contourParams.Na = Na_spin_->value();
    config_.contourParams.Ns = Ns_spin_->value();
    config_.contourParams.cycle2Enabled = internalspeed_groupbox_->isChecked();
    config_.contourParams.diskRadius = disk_radius_spin_->value();

    config_.regionParams.lambdaOut = lambda_out_spin_->value();
    config_.regionParams.lambdaIn = lambda_in_spin_->value();

    config_.regionParams.colorSpace =
        color_space_cb_->currentData().value<fluvel_ip::ColorSpaceOption>();

    config_.regionParams.weights.c1 = alpha_spin_->value();
    config_.regionParams.weights.c2 = beta_spin_->value();
    config_.regionParams.weights.c3 = gamma_spin_->value();
}

void AlgoSettingsWidget::reject()
{
    int index = connectivity_cb_->findData(QVariant::fromValue(config_.connectivity));
    if (index >= 0)
        connectivity_cb_->setCurrentIndex(index);

    Na_spin_->setValue(config_.contourParams.Na);
    Ns_spin_->setValue(config_.contourParams.Ns);
    internalspeed_groupbox_->setChecked(config_.contourParams.cycle2Enabled);
    disk_radius_spin_->setValue(config_.contourParams.diskRadius);

    lambda_out_spin_->setValue(config_.regionParams.lambdaOut);
    lambda_in_spin_->setValue(config_.regionParams.lambdaIn);

    index = color_space_cb_->findData(QVariant::fromValue(config_.regionParams.colorSpace));
    if (index >= 0)
        color_space_cb_->setCurrentIndex(index);

    alpha_spin_->setValue(config_.regionParams.weights.c1);
    beta_spin_->setValue(config_.regionParams.weights.c2);
    gamma_spin_->setValue(config_.regionParams.weights.c3);
}

} // namespace fluvel_app
