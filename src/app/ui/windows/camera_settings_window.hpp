#ifndef CAMERA_SETTINGS_WINDOW_HPP
#define CAMERA_SETTINGS_WINDOW_HPP

#include <QDialog>

namespace ofeli_app
{

class CameraSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    CameraSettingsWindow(QWidget* parent);
};

}

#endif // CAMERA_SETTINGS_WINDOW_HPP
