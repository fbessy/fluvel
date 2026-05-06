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
    connectivityCb_ = new QComboBox;

    connectivityCb_->addItem(tr("4-connected (Von Neumann)"),
                             QVariant::fromValue(fluvel_ip::Connectivity::Four));

    connectivityCb_->addItem(tr("8-connected (Moore)"),
                             QVariant::fromValue(fluvel_ip::Connectivity::Eight));

    connectivityCb_->setToolTip(tr("Defines the neighborhood used for contour connectivity "
                                   "(4-connected or 8-connected)."));

    QFormLayout* connect_layout = new QFormLayout;
    connect_layout->addRow(tr("Connectivity: "), connectivityCb_);

    QGroupBox* externalspeed_groupbox = new QGroupBox(tr("Cycle 1 – Data-driven evolution"));

    naSpin_ = new QSpinBox;
    naSpin_->setSingleStep(1);
    naSpin_->setMinimum(1);
    naSpin_->setMaximum(999);
    naSpin_->setSuffix(tr(" iterations"));
    naSpin_->setToolTip(tr("Number of iterations of the data-driven evolution (Cycle 1)."));
    QFormLayout* Na_layout = new QFormLayout;
    Na_layout->addRow("Na =", naSpin_);

    lambdaOutSpin_ = new QSpinBox;
    lambdaOutSpin_->setSingleStep(1);
    lambdaOutSpin_->setMinimum(1);
    lambdaOutSpin_->setMaximum(100000);
    lambdaOutSpin_->setToolTip(tr("weight of the outside homogeneity criterion"));

    lambdaInSpin_ = new QSpinBox;
    lambdaInSpin_->setSingleStep(1);
    lambdaInSpin_->setMinimum(1);
    lambdaInSpin_->setMaximum(100000);
    lambdaInSpin_->setToolTip(tr("weight of the inside homogeneity criterion"));

    QFormLayout* lambda_layout = new QFormLayout;

    lambda_layout->addRow("λout =", lambdaOutSpin_);
    lambda_layout->addRow("λin =", lambdaInSpin_);

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addLayout(lambda_layout);

    QHBoxLayout* speed_layout = new QHBoxLayout;
    speed_layout->addLayout(chanvese_layout);

    colorWeightsGroupbox_ = new QGroupBox(tr("Color space"));
    colorWeightsGroupbox_->setFlat(true);

    colorSpaceCb_ = new QComboBox;
    colorSpaceCb_->addItem("RGB", QVariant::fromValue(fluvel_ip::ColorSpaceOption::RGB));

    colorSpaceCb_->addItem("YUV", QVariant::fromValue(fluvel_ip::ColorSpaceOption::YUV));

    colorSpaceCb_->addItem("L*a*b* (CIELAB)",
                             QVariant::fromValue(fluvel_ip::ColorSpaceOption::Lab));

    colorSpaceCb_->addItem("L*u*v* (CIELUV)",
                             QVariant::fromValue(fluvel_ip::ColorSpaceOption::Luv));

    alphaSpin_ = new QSpinBox;
    alphaSpin_->setSingleStep(1);
    alphaSpin_->setMinimum(1);
    alphaSpin_->setMaximum(100000);

    betaSpin_ = new QSpinBox;
    betaSpin_->setSingleStep(1);
    betaSpin_->setMinimum(1);
    betaSpin_->setMaximum(100000);

    gammaSpin_ = new QSpinBox;
    gammaSpin_->setSingleStep(1);
    gammaSpin_->setMinimum(1);
    gammaSpin_->setMaximum(100000);

    QFormLayout* color_weights_layout = new QFormLayout;
    color_weights_layout->addRow(tr("1st component weight ="), alphaSpin_);
    color_weights_layout->addRow(tr("2nd component weight ="), betaSpin_);
    color_weights_layout->addRow(tr("3rd component weight ="), gammaSpin_);

    QVBoxLayout* vcolor_layout = new QVBoxLayout;
    vcolor_layout->addWidget(colorSpaceCb_);
    vcolor_layout->addLayout(color_weights_layout);

    colorWeightsGroupbox_->setLayout(vcolor_layout);

    QVBoxLayout* externalspeed_layout = new QVBoxLayout;

    externalspeed_layout->addLayout(Na_layout);
    externalspeed_layout->addLayout(speed_layout);
    externalspeed_layout->addWidget(colorWeightsGroupbox_);
    externalspeed_groupbox->setLayout(externalspeed_layout);

    ////////////////////////////////////////////

    internalspeedGroupbox_ = new QGroupBox(tr("Cycle 2 - Internal smoothing"));
    internalspeedGroupbox_->setCheckable(true);
    internalspeedGroupbox_->setChecked(true);

    nsSpin_ = new QSpinBox;
    nsSpin_->setSingleStep(1);
    nsSpin_->setMinimum(1);
    nsSpin_->setMaximum(999);
    nsSpin_->setSuffix(tr(" iterations"));
    nsSpin_->setToolTip(tr("Number of internal smoothing iterations (Cycle 2)."));

    diskRadiusSpin_ = new QSpinBox;
    diskRadiusSpin_->setSingleStep(1);
    diskRadiusSpin_->setMinimum(1);
    diskRadiusSpin_->setMaximum(999);
    diskRadiusSpin_->setToolTip(tr("Radius of the disk-shaped neighborhood used for the "
                                     "majority vote during internal smoothing."));

    QFormLayout* internalspeed_layout = new QFormLayout;
    internalspeed_layout->addRow("Ns =", nsSpin_);
    internalspeed_layout->addRow("R =", diskRadiusSpin_);

    internalspeedGroupbox_->setLayout(internalspeed_layout);

    ////////////////////////////////////////////

    QVBoxLayout* algorithm_layout = new QVBoxLayout;
    algorithm_layout->setContentsMargins(0, 0, 0, 0);
    algorithm_layout->addLayout(connect_layout);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(externalspeed_groupbox);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(internalspeedGroupbox_);
    algorithm_layout->addStretch(1);

    setLayout(algorithm_layout);

    // to initialize ui state in function of the configuration
    reject();

    connect(connectivityCb_, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
                auto connectivity =
                    connectivityCb_->itemData(index).value<fluvel_ip::Connectivity>();

                emit connectivityChanged(connectivity);
            });
}

void AlgoSettingsWidget::accept()
{
    config_.connectivity = connectivityCb_->currentData().value<fluvel_ip::Connectivity>();

    config_.contourParams.Na = naSpin_->value();
    config_.contourParams.Ns = nsSpin_->value();
    config_.contourParams.cycle2Enabled = internalspeedGroupbox_->isChecked();
    config_.contourParams.diskRadius = diskRadiusSpin_->value();

    config_.regionParams.lambdaOut = lambdaOutSpin_->value();
    config_.regionParams.lambdaIn = lambdaInSpin_->value();

    config_.regionParams.colorSpace =
        colorSpaceCb_->currentData().value<fluvel_ip::ColorSpaceOption>();

    config_.regionParams.weights.c1 = alphaSpin_->value();
    config_.regionParams.weights.c2 = betaSpin_->value();
    config_.regionParams.weights.c3 = gammaSpin_->value();
}

void AlgoSettingsWidget::reject()
{
    int index = connectivityCb_->findData(QVariant::fromValue(config_.connectivity));
    if (index >= 0)
        connectivityCb_->setCurrentIndex(index);

    naSpin_->setValue(config_.contourParams.Na);
    nsSpin_->setValue(config_.contourParams.Ns);
    internalspeedGroupbox_->setChecked(config_.contourParams.cycle2Enabled);
    diskRadiusSpin_->setValue(config_.contourParams.diskRadius);

    lambdaOutSpin_->setValue(config_.regionParams.lambdaOut);
    lambdaInSpin_->setValue(config_.regionParams.lambdaIn);

    index = colorSpaceCb_->findData(QVariant::fromValue(config_.regionParams.colorSpace));
    if (index >= 0)
        colorSpaceCb_->setCurrentIndex(index);

    alphaSpin_->setValue(config_.regionParams.weights.c1);
    betaSpin_->setValue(config_.regionParams.weights.c2);
    gammaSpin_->setValue(config_.regionParams.weights.c3);
}

} // namespace fluvel_app
