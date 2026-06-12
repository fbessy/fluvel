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

namespace fluvel
{

class AlgoSettingsWidget;

/**
 * @brief Dialog for configuring video session and processing settings.
 *
 * This dialog allows the user to configure parameters related to
 * video reception and processing, including:
 * - downscaling options
 * - spatial and temporal processing
 * - algorithm parameters via AlgoSettingsWidget
 *
 * The configuration is applied when the dialog is accepted.
 */
class VideoSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the video settings dialog.
     * @param config Initial video compute configuration.
     * @param parent Optional parent widget.
     *
     */
    VideoSettingsWindow(const VideoComputeConfig& config, QWidget* parent = nullptr);

signals:
    /**
     * @brief Emitted when the configuration is accepted.
     *
     * @param config Updated video session configuration.
     */
    void videoComputeSettingsAccepted(const fluvel::VideoComputeConfig& config);

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
    VideoComputeConfig committedConfig_{}; // applied settings
    VideoComputeConfig editedConfig_{};    // pending changes
};

} // namespace fluvel
