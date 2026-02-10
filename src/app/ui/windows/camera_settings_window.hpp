#ifndef CAMERA_SETTINGS_WINDOW_HPP
#define CAMERA_SETTINGS_WINDOW_HPP

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

private:

    void setupUiDownscaleGb();

    QDialogButtonBox* dial_buttons;
    QTabWidget* tabs;
    AlgoSettingsWidget* algoWidget;
    QSpinBox* phases_sb;

    QGroupBox* downscale_gb;
    QComboBox* downscale_factor_cb;
    QCheckBox* filter_cb;

};

}

#endif // CAMERA_SETTINGS_WINDOW_HPP
