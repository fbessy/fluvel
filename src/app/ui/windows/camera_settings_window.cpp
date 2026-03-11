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
#include <QTabBar>
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

    dial_buttons_ = new QDialogButtonBox(this);
    dial_buttons_->addButton(QDialogButtonBox::Ok);
    dial_buttons_->addButton(QDialogButtonBox::Cancel);
    dial_buttons_->addButton(QDialogButtonBox::RestoreDefaults);

    tabs_ = new QTabWidget(this);

    auto* tabBar = tabs_->tabBar();

    tabBar->setExpanding(false);
    tabBar->setUsesScrollButtons(false);
    tabBar->setElideMode(Qt::ElideNone);

    setupUiDownscaleGb();

    filter_cb_ = new QCheckBox(tr("Motion-Adaptive Smoothing"));

    auto* preprocess_layout = new QVBoxLayout(this);
    preprocess_layout->addWidget(downscale_gb_);
    preprocess_layout->addWidget(filter_cb_);
    preprocess_layout->addStretch(1);

    auto* preprocess_gb = new QGroupBox;
    preprocess_gb->setLayout(preprocess_layout);

    algoWidget_ = new AlgoSettingsWidget(config_.compute.algo, this);

    phases_sb_ = new QSpinBox;
    phases_sb_->setMinimum(1);

    auto* fl = new QFormLayout;
    fl->addRow(tr("phases (cycle 1 to 2) per frame"), phases_sb_);

    auto* algoLayout = new QVBoxLayout;
    algoLayout->addWidget(algoWidget_);
    algoLayout->addSpacing(8);
    algoLayout->addLayout(fl);
    algoLayout->addStretch(1);

    auto* algo_gb = new QGroupBox;
    algo_gb->setLayout(algoLayout);

    tabs_->addTab(preprocess_gb, tr("Preprocessing"));
    tabs_->addTab(algo_gb, tr("Algorithm"));

    auto* layout = new QVBoxLayout;
    layout->addWidget(tabs_);
    layout->addWidget(dial_buttons_);

    setLayout(layout);

    updateUIFromConfig();

    connect(dial_buttons_, &QDialogButtonBox::accepted, this, &CameraSettingsWindow::accept);

    connect(dial_buttons_, &QDialogButtonBox::rejected, this, &CameraSettingsWindow::reject);

    connect(dial_buttons_->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this,
            &CameraSettingsWindow::restoreToDefaults);
}

void CameraSettingsWindow::setupUiDownscaleGb()
{
    downscale_gb_ = new QGroupBox(tr("Downscale"));
    downscale_gb_->setCheckable(true);

    downscale_factor_cb_ = new QComboBox;
    downscale_factor_cb_->addItem("2", 2);
    downscale_factor_cb_->addItem("4", 4);

    auto* fl = new QFormLayout;
    fl->addRow(tr("Factor:"), downscale_factor_cb_);

    downscale_gb_->setLayout(fl);
}

void CameraSettingsWindow::accept()
{
    config_.compute.downscale.hasDownscale = downscale_gb_->isChecked();
    config_.compute.downscale.downscaleFactor = downscale_factor_cb_->currentData().toInt();
    config_.compute.hasTemporalFiltering = filter_cb_->isChecked();

    algoWidget_->accept();
    config_.compute.cyclesNbr = phases_sb_->value();

    emit settingsAccepted(config_);

    QDialog::accept();
}

void CameraSettingsWindow::updateUIFromConfig()
{
    QSignalBlocker blocker(this);

    downscale_gb_->setChecked(config_.compute.downscale.hasDownscale);

    int index = downscale_factor_cb_->findData(config_.compute.downscale.downscaleFactor);
    if (index >= 0)
        downscale_factor_cb_->setCurrentIndex(index);

    filter_cb_->setChecked(config_.compute.hasTemporalFiltering);

    algoWidget_->reject();
    phases_sb_->setValue(config_.compute.cyclesNbr);
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
