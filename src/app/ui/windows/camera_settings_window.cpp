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

CameraSettingsWindow::CameraSettingsWindow(const VideoSessionSettings& config, QWidget* parent)
    : QDialog(parent)
    , committedConfig_(config.compute)
    , editedConfig_(config.compute)
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

    spatialCb_ = new QCheckBox(tr("Spatial Smoothing (3×3)"));
    temporalCb_ = new QCheckBox(tr("Motion-Adaptive Smoothing"));

    auto* preprocessLayout = new QVBoxLayout(this);
    preprocessLayout->addWidget(downscaleGb_);
    preprocessLayout->addWidget(spatialCb_);
    preprocessLayout->addWidget(temporalCb_);
    preprocessLayout->addStretch(1);

    auto* preprocessGb = new QGroupBox;
    preprocessGb->setLayout(preprocessLayout);

    algoWidget_ = new AlgoSettingsWidget(editedConfig_.contourConfig, this);

    auto* algoLayout = new QVBoxLayout;
    algoLayout->addWidget(algoWidget_);

    auto* algo_gb = new QGroupBox;
    algo_gb->setLayout(algoLayout);

    tabs_->addTab(preprocessGb, tr("Preprocess"));
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
    fl->addRow(tr("Factor: "), downscaleFactorCb_);

    downscaleGb_->setLayout(fl);
}

void CameraSettingsWindow::accept()
{
    editedConfig_.downscale.downscaleEnabled = downscaleGb_->isChecked();
    editedConfig_.downscale.downscaleFactor = downscaleFactorCb_->currentData().toInt();
    editedConfig_.spatialFilteringEnabled = spatialCb_->isChecked();
    editedConfig_.temporalFilteringEnabled = temporalCb_->isChecked();

    algoWidget_->accept();

    committedConfig_ = editedConfig_;
    emit videoComputeSettingsAccepted(committedConfig_);

    QDialog::accept();
}

void CameraSettingsWindow::updateUIFromConfig()
{
    QSignalBlocker blocker(this);

    downscaleGb_->setChecked(editedConfig_.downscale.downscaleEnabled);

    int index = downscaleFactorCb_->findData(editedConfig_.downscale.downscaleFactor);
    if (index >= 0)
        downscaleFactorCb_->setCurrentIndex(index);

    spatialCb_->setChecked(editedConfig_.spatialFilteringEnabled);
    temporalCb_->setChecked(editedConfig_.temporalFilteringEnabled);

    algoWidget_->reject();
}

void CameraSettingsWindow::reject()
{
    editedConfig_ = committedConfig_;
    updateUIFromConfig();

    QDialog::reject();
}

void CameraSettingsWindow::restoreToDefaults()
{
    editedConfig_ = {};   // reset avec defaults structs
    updateUIFromConfig(); // resync UI
}

void CameraSettingsWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("ui_geometry/camera_settings_window", saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace fluvel_app
