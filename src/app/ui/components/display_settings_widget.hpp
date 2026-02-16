#ifndef DISPLAY_SETTINGS_WIDGET_HPP
#define DISPLAY_SETTINGS_WIDGET_HPP

#include "common_settings.hpp"

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

namespace ofeli_app
{

class ColorSelectorWidget;

class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DisplaySettingsWidget(QWidget* parent,
                          Session session);

private slots:
    void onImgSettingsChanged();
    void onCamSettingsChanged();

private:

    void setConfig();
    void refresh_input_displayed_cb_availability();

    ColorSelectorWidget* lout_selector_;
    ColorSelectorWidget* lin_selector_;

    QCheckBox* display_overlay_cb_;
    QCheckBox* input_displayed_cb_;
    QCheckBox* flip_cb_;

    DisplayConfig config_;
    Session session_;
};

}

#endif // DISPLAY_SETTINGS_WIDGET_HPP
