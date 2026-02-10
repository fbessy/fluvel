#include "algo_settings_widget.hpp"
#include "application_settings.hpp"

#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QFormLayout>

namespace ofeli_app
{

AlgoSettingsWidget::AlgoSettingsWidget(QWidget* parent,
                                       Session session)
    : QWidget(parent)
{
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

    auto& config = AppSettings::instance().imgSessSettings.img_disp_conf;

    QColor RGBout_list( get_QRgb(config.l_out_color) );
    QColor RGBin_list( get_QRgb(config.l_in_color) );

    lambda_layout->addRow("<font color="+RGBout_list.name()+">"+"λout"+"<font color=black>"+" =", lambda_out_spin);
    lambda_layout->addRow("<font color="+RGBin_list.name()+">"+"λin"+"<font color=black>"+" =", lambda_in_spin);

    QVBoxLayout* chanvese_layout = new QVBoxLayout;
    chanvese_layout->addLayout(lambda_layout);



    QHBoxLayout* speed_layout = new QHBoxLayout;
    speed_layout->addLayout(chanvese_layout);

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

    beta_spin = new QSpinBox;
    beta_spin->setSingleStep(1);
    beta_spin->setMinimum(1);
    beta_spin->setMaximum(100000);

    gamma_spin = new QSpinBox;
    gamma_spin->setSingleStep(1);
    gamma_spin->setMinimum(1);
    gamma_spin->setMaximum(100000);



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

    QVBoxLayout* algorithm_layout = new QVBoxLayout;
    algorithm_layout->setContentsMargins(0, 0, 0, 0);
    algorithm_layout->addLayout(connect_layout);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(externalspeed_groupbox);
    algorithm_layout->addSpacing(8);
    algorithm_layout->addWidget(internalspeed_groupbox);
    algorithm_layout->addStretch(1);

    setLayout(algorithm_layout);
}

}
