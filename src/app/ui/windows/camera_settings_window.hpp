#ifndef CAMERA_SETTINGS_WINDOW_HPP
#define CAMERA_SETTINGS_WINDOW_HPP

#include "application_settings.hpp"

#include <QDialog>

class QDialogButtonBox;
class QTabWidget;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QSpinBox;

namespace ofeli_app
{

class AlgoSettingsWidget;

class CameraSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    CameraSettingsWindow(QWidget* parent);

protected:

    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

private:

    void setupUiDownscaleGb();

    QDialogButtonBox* dial_buttons;
    QTabWidget* tabs;
    AlgoSettingsWidget* algoWidget;
    QSpinBox* phases_sb;

    QGroupBox* downscale_gb;
    QComboBox* downscale_factor_cb;
    QCheckBox* filter_cb;

    CameraSessionSettings& config_;
};

}

#endif // CAMERA_SETTINGS_WINDOW_HPP
