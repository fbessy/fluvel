#include "camera_settings_window.hpp"
#include "algo_settings_widget.hpp"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QFormLayout>

namespace ofeli_app
{

CameraSettingsWindow::CameraSettingsWindow(QWidget* parent):
    QDialog(parent)
{
    setWindowTitle(tr("Camera session settings"));

    dial_buttons = new QDialogButtonBox(this);
    dial_buttons->addButton(QDialogButtonBox::Ok);
    dial_buttons->addButton(QDialogButtonBox::Cancel);
    dial_buttons->addButton(QDialogButtonBox::Reset);

    tabs = new QTabWidget(this);

    auto *tabBar = tabs->tabBar();

    tabBar->setExpanding(false);
    tabBar->setUsesScrollButtons(false);
    tabBar->setElideMode(Qt::ElideNone);

    setupUiDownscaleGb();

    filter_cb = new QCheckBox(tr("Temporal filtering"));

    auto *preprocess_layout = new QVBoxLayout(this);
    preprocess_layout->addWidget(downscale_gb);
    preprocess_layout->addWidget(filter_cb);
    preprocess_layout->addStretch(1);

    auto * preprocess_gb = new QGroupBox;
    preprocess_gb->setLayout(preprocess_layout);


    algoWidget = new AlgoSettingsWidget(this,
                                        Session::Camera);

    phases_sb = new QSpinBox;
    phases_sb->setMinimum(1);

    auto* fl = new QFormLayout;
    fl->addRow(tr("phases (cycle 1 to 2) per frame"), phases_sb);

    auto* algoLayout = new QVBoxLayout;
    algoLayout->addWidget(algoWidget);
    algoLayout->addSpacing(8);
    algoLayout->addLayout(fl);
    algoLayout->addStretch(1);

    auto* algo_gb = new QGroupBox;
    algo_gb->setLayout(algoLayout);

    tabs->addTab( preprocess_gb, tr("Preprocessing") );
    tabs->addTab( algo_gb, tr("Algorithm") );

    auto* layout = new QVBoxLayout;
    layout->addWidget(tabs);
    layout->addWidget(dial_buttons);

    setLayout(layout);
}

void CameraSettingsWindow::setupUiDownscaleGb()
{
    downscale_gb = new QGroupBox(tr("Downscale"));
    downscale_gb->setCheckable(true);

    downscale_factor_cb = new QComboBox;
    downscale_factor_cb->addItem("2");
    downscale_factor_cb->addItem("4");

    auto* fl = new QFormLayout;
    fl->addRow(tr("Factor:"),
               downscale_factor_cb);

    downscale_gb->setLayout(fl);
}

}
