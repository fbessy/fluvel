// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QWidget>

class QComboBox;
class QSpinBox;
class QGroupBox;

namespace fluvel_app
{

/**
 * @brief Widget for configuring active contour algorithm parameters.
 *
 * This widget exposes UI controls to edit an ActiveContourConfig,
 * including:
 * - connectivity
 * - iteration counts (Na, Ns)
 * - data terms (lambda in/out)
 * - color model and weights
 * - internal smoothing parameters
 *
 * Changes can be applied or discarded via accept() / reject().
 *
 * @note The configuration is modified by reference and must remain valid
 *       for the lifetime of the widget.
 *
 */
class AlgoSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the algorithm settings widget.
     *      * @param config Configuration object to edit (by reference).
     * @param parent Optional parent widget.
     */
    AlgoSettingsWidget(ActiveContourConfig& config, QWidget* parent = nullptr);

    /**
     * @brief Applies the current UI values to the configuration.
     */
    void accept();

    /**
     * @brief Restores UI values from the current configuration.
     */
    void reject();

signals:
    /**
     * @brief Emitted when the connectivity setting changes.
     *      * @param connectivity Selected connectivity mode.
     */
    void connectivityChanged(fluvel_ip::Connectivity connectivity);

private:
    /// UI elements for connectivity selection.
    QComboBox* connectivityCb_ = nullptr;

    /// Iteration parameters.
    QSpinBox* naSpin_ = nullptr;
    QSpinBox* nsSpin_ = nullptr;

    /// Data term parameters.
    QSpinBox* lambdaOutSpin_ = nullptr;
    QSpinBox* lambdaInSpin_ = nullptr;

    /// Color model parameters.
    QGroupBox* colorWeightsGroupbox_ = nullptr;
    QComboBox* colorSpaceCb_ = nullptr;
    QSpinBox* alphaSpin_ = nullptr;
    QSpinBox* betaSpin_ = nullptr;
    QSpinBox* gammaSpin_ = nullptr;
    int alpha_{0};
    int beta_{0};
    int gamma_{0};

    /// Internal smoothing parameters.
    QGroupBox* internalspeedGroupbox_ = nullptr;
    QSpinBox* diskRadiusSpin_ = nullptr;

    ActiveContourConfig& config_;
};

} // namespace fluvel_app
