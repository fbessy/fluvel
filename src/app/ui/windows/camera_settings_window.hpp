// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"

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

class CameraSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    CameraSettingsWindow(QWidget* parent, const VideoSessionSettings& config);

signals:
    void videoSessionSettingsAccepted(const fluvel_app::VideoSessionSettings& config);

protected:
    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

    void closeEvent(QCloseEvent* event) override;

private:
    // --- Setup ---

    void setupDownscaleGroup();
    void updateUIFromConfig();
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
