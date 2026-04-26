// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QDialog>

class QWidget;
class QCloseEvent;

class QDialogButtonBox;
class QTabWidget;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QSpinBox;

namespace fluvel_app
{

class AlgoSettingsWidget;

/**
 * @brief Dialog for configuring video session and processing settings.
 *
 * This dialog allows the user to configure parameters related to
 * video acquisition and processing, including:
 * - downscaling options
 * - spatial and temporal processing
 * - algorithm parameters via AlgoSettingsWidget
 *
 * The configuration is applied when the dialog is accepted.
 */
class CameraSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the camera settings dialog.
     * @param config Initial video session configuration.
     * @param parent Optional parent widget.
     *
     */
    CameraSettingsWindow(const VideoSessionSettings& config, QWidget* parent = nullptr);

signals:
    /**
     * @brief Emitted when the configuration is accepted.
     *      * @param config Updated video session configuration.
     */
    void videoSessionSettingsAccepted(const fluvel_app::VideoSessionSettings& config);

protected:
    /**
     * @brief Applies the current UI values and emits the updated configuration.
     */
    void accept() override;

    /**
     * @brief Restores UI values from the current configuration.
     */
    void reject() override;

    /**
     * @brief Handles dialog close events.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    // --- Setup ---

    /// Initializes the downscale configuration UI.
    void setupDownscaleGroup();

    /// Updates UI elements from the current configuration.
    void updateUIFromConfig();

    /// Restores default configuration values.
    void restoreToDefaults();

    // --- UI ---

    QTabWidget* tabs_ = nullptr;

    QGroupBox* downscaleGb_ = nullptr;
    QComboBox* downscaleFactorCb_ = nullptr;

    QCheckBox* spatialCb_ = nullptr;
    QCheckBox* temporalCb_ = nullptr;

    AlgoSettingsWidget* algoWidget_ = nullptr;

    QDialogButtonBox* dialogButtons_ = nullptr;

    // --- Model ---
    VideoSessionSettings config_;
};

} // namespace fluvel_app
