// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

namespace fluvel_app
{

class ColorSelectorWidget;

/**
 * @brief Widget for configuring display parameters of the image viewer.
 *
 * This widget provides UI controls to adjust how images and overlays
 * are rendered, including:
 * - pipeline source selection (raw / preprocessed)
 * - contour colors
 * - display options (flip, smoothing, overlay)
 *
 * It emits displayConfigChanged() whenever the configuration is updated.
 */
class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the display settings widget.
     *      * @param config Initial display configuration.
     * @param parent Optional parent widget.
     */
    DisplaySettingsWidget(const DisplayConfig& config, QWidget* parent = nullptr);

    /**
     * @brief Enables or disables pipeline-related options.
     *      * @param hasPreprocessing True if preprocessing is available.
     */
    void updatePipelineAvailability(bool hasPreprocessing);

signals:
    /**
     * @brief Emitted when the display configuration changes.
     *      * @param config Updated configuration.
     */
    void displayConfigChanged(const fluvel_app::DisplayConfig& config);

public slots:
    /**
     * @brief Shows or hides the settings panel.
     *      * @param open True to show the panel, false to hide it.
     */
    void setPanelVisible(bool open);

private:
    /**
     * @brief Animates panel visibility changes.
     */
    void animate(bool open);

    bool isAnimating_{false};

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
