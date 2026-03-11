// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

namespace fluvel_app
{

class ColorSelectorWidget;

class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DisplaySettingsWidget(const DisplayConfig& config, QWidget* parent = nullptr);
    void updatePipelineAvailability(bool hasPreprocessing);

signals:
    void displayConfigChanged(const fluvel_app::DisplayConfig& config);

public slots:
    void setPanelVisible(bool open);

private:
    void animate(bool open);
    bool isAnimating_ = false;

    QGroupBox* pipeline_displayed_gb_;
    QRadioButton* source_rb_;
    QRadioButton* preprocessed_rb_;

    ColorSelectorWidget* lout_selector_;
    ColorSelectorWidget* lin_selector_;

    QCheckBox* flip_cb_;
    QCheckBox* smooth_cb_;

    QCheckBox* display_overlay_cb_;

    DisplayConfig config_;
};

} // namespace fluvel_app
