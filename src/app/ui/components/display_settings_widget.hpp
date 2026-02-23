#ifndef DISPLAY_SETTINGS_WIDGET_HPP
#define DISPLAY_SETTINGS_WIDGET_HPP

#include "common_settings.hpp"

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>

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
    void onVideoSettingsChanged();

private:

    void setConfig();
    void refresh_pipeline_displayed_gb_availability();

    ColorSelectorWidget* lout_selector_;
    ColorSelectorWidget* lin_selector_;

    QCheckBox* display_overlay_cb_;
    QRadioButton* source_rb_;
    QRadioButton* preprocessed_rb_;

    QCheckBox* flip_cb_;
    QCheckBox* smooth_cb_;

    DisplayConfig config_;
    Session session_;
};

}

#endif // DISPLAY_SETTINGS_WIDGET_HPP
