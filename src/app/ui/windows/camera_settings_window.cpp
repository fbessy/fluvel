// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_settings_window.hpp"
#include "algo_settings_widget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace fluvel_app
{

CameraSettingsWindow::CameraSettingsWindow(QWidget* parent, const VideoSessionSettings& config)
    : QDialog(parent)
    , config_(config)
{
    setWindowTitle(tr("Camera session settings"));

    QSettings settings;

    if (settings.contains("ui_geometry/camera_settings_window"))
    {
        restoreGeometry(settings.value("ui_geometry/camera_settings_window").toByteArray());
    }
    else
    {
        resize(350, 650);
    }

    dialogButtons_ = new QDialogButtonBox(this);
    dialogButtons_->addButton(QDialogButtonBox::Ok);
    dialogButtons_->addButton(QDialogButtonBox::Cancel);
    dialogButtons_->addButton(QDialogButtonBox::RestoreDefaults);

    tabs_ = new QTabWidget(this);

    auto* tabBar = tabs_->tabBar();

    tabBar->setExpanding(false);
    tabBar->setUsesScrollButtons(false);
    tabBar->setElideMode(Qt::ElideNone);

    setupDownscaleGroup();

    filterCb_ = new QCheckBox(tr("Motion-Adaptive Smoothing"));

    auto* preprocess_layout = new QVBoxLayout(this);
    preprocess_layout->addWidget(downscaleGb_);
    preprocess_layout->addWidget(filterCb_);
    preprocess_layout->addStretch(1);

    auto* preprocess_gb = new QGroupBox;
    preprocess_gb->setLayout(preprocess_layout);

    algoWidget_ = new AlgoSettingsWidget(config_.compute.algo, this);

    auto* algoLayout = new QVBoxLayout;
    algoLayout->addWidget(algoWidget_);
    algoLayout->addSpacing(8);
    algoLayout->addStretch(1);

    auto* algo_gb = new QGroupBox;
    algo_gb->setLayout(algoLayout);

    tabs_->addTab(preprocess_gb, tr("Preprocessing"));
    tabs_->addTab(algo_gb, tr("Algorithm"));

    auto* layout = new QVBoxLayout;
    layout->addWidget(tabs_);
    layout->addWidget(dialogButtons_);

    setLayout(layout);

    updateUIFromConfig();

    connect(dialogButtons_, &QDialogButtonBox::accepted, this, &CameraSettingsWindow::accept);

    connect(dialogButtons_, &QDialogButtonBox::rejected, this, &CameraSettingsWindow::reject);

    connect(dialogButtons_->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this,
            &CameraSettingsWindow::restoreToDefaults);
}

void CameraSettingsWindow::setupDownscaleGroup()
{
    downscaleGb_ = new QGroupBox(tr("Downscale"));
    downscaleGb_->setCheckable(true);

    downscaleFactorCb_ = new QComboBox;
    downscaleFactorCb_->addItem("2", 2);
    downscaleFactorCb_->addItem("4", 4);

    auto* fl = new QFormLayout;
    fl->addRow(tr("Factor:"), downscaleFactorCb_);

    downscaleGb_->setLayout(fl);
}

void CameraSettingsWindow::accept()
{
    config_.compute.downscale.hasDownscale = downscaleGb_->isChecked();
    config_.compute.downscale.downscaleFactor = downscaleFactorCb_->currentData().toInt();
    config_.compute.hasTemporalFiltering = filterCb_->isChecked();

    algoWidget_->accept();

    emit videoSessionSettingsAccepted(config_);

    QDialog::accept();
}

void CameraSettingsWindow::updateUIFromConfig()
{
    QSignalBlocker blocker(this);

    downscaleGb_->setChecked(config_.compute.downscale.hasDownscale);

    int index = downscaleFactorCb_->findData(config_.compute.downscale.downscaleFactor);
    if (index >= 0)
        downscaleFactorCb_->setCurrentIndex(index);

    filterCb_->setChecked(config_.compute.hasTemporalFiltering);

    algoWidget_->reject();
}

void CameraSettingsWindow::reject()
{
    updateUIFromConfig();

    QDialog::reject();
}

void CameraSettingsWindow::restoreToDefaults()
{
    // config_ = CamSessionConfig::defaults();
    updateUIFromConfig();
}

void CameraSettingsWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("ui_geometry/camera_settings_window", saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace fluvel_app
