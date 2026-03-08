// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"

#include <QWidget>

class QComboBox;
class QSpinBox;
class QGroupBox;

namespace fluvel_app
{

class AlgoSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    AlgoSettingsWidget(QWidget* parent, Session session);

    void accept();
    void reject();

private:
    static AlgoConfig& config(Session session);

    QComboBox* connectivity_cb_;

    QSpinBox* Na_spin_;
    QSpinBox* Ns_spin_;
    QSpinBox* lambda_out_spin_;
    QSpinBox* lambda_in_spin_;
    QGroupBox* color_weights_groupbox_;
    QComboBox* color_space_cb_;
    QSpinBox* alpha_spin_;
    QSpinBox* beta_spin_;
    QSpinBox* gamma_spin_;
    int alpha_;
    int beta_;
    int gamma_;

    QGroupBox* internalspeed_groupbox_;
    QSpinBox* disk_radius_spin_;

    Session session_;
    AlgoConfig& config_;
};

} // namespace fluvel_app
