// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings.hpp"

#include <QDialog>

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
    void setupUiDownscaleGb();
    void updateUIFromConfig();

    void restoreToDefaults();

    QDialogButtonBox* dial_buttons_;
    QTabWidget* tabs_;
    AlgoSettingsWidget* algoWidget_;
    QSpinBox* phases_sb_;

    QGroupBox* downscale_gb_;
    QComboBox* downscale_factor_cb_;
    QCheckBox* filter_cb_;

    VideoSessionSettings config_;
};

} // namespace fluvel_app
