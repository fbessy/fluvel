#ifndef ALGO_SETTINGS_WIDGET_HPP
#define ALGO_SETTINGS_WIDGET_HPP

#include "common_settings.hpp"

#include <QWidget>

class QComboBox;
class QSpinBox;
class QGroupBox;

namespace ofeli_app
{

class AlgoSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    AlgoSettingsWidget(QWidget* parent,
                       Session session);

private:

    QComboBox* connectivity_cb;

    QSpinBox* Na_spin;
    QSpinBox* Ns_spin;
    QSpinBox* lambda_out_spin;
    QSpinBox* lambda_in_spin;
    QGroupBox* color_weights_groupbox;
    QComboBox* color_space_cb;
    QSpinBox* alpha_spin;
    QSpinBox* beta_spin;
    QSpinBox* gamma_spin;
    int alpha;
    int beta;
    int gamma;

    QGroupBox* internalspeed_groupbox;
    QSpinBox* disk_radius_spin;

};

}

#endif // ALGO_SETTINGS_WIDGET_HPP
