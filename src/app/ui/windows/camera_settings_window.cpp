#include "camera_settings_window.hpp"
#include "algo_settings_widget.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

namespace ofeli_app
{

CameraSettingsWindow::CameraSettingsWindow(QWidget* parent)
    : QDialog(parent)
    , config_(AppSettings::instance().camConfig)
{
    setWindowTitle(tr("Camera session settings"));

    dial_buttons_ = new QDialogButtonBox(this);
    dial_buttons_->addButton(QDialogButtonBox::Ok);
    dial_buttons_->addButton(QDialogButtonBox::Cancel);
    dial_buttons_->addButton(QDialogButtonBox::Reset);

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

    algoWidget_ = new AlgoSettingsWidget(this, Session::Camera);

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

    // for data persistence
    AppSettings::instance().save_cam_session_config();

    QDialog::accept();
}

void CameraSettingsWindow::updateUIFromConfig()
{
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

} // namespace ofeli_app
