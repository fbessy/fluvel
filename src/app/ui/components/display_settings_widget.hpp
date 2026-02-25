#ifndef DISPLAY_SETTINGS_WIDGET_HPP
#define DISPLAY_SETTINGS_WIDGET_HPP

#include "common_settings.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

namespace ofeli_app
{

class ColorSelectorWidget;

class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DisplaySettingsWidget(QWidget* parent, Session session);

public slots:
    void setPanelVisible(bool open);

private slots:
    void onImgSettingsChanged();
    void onVideoSettingsChanged();

private:
    void setConfig();
    void refresh_pipeline_displayed_gb_availability();

    void animate(bool open);
    bool isAnimating_ = false;

    QGroupBox* pipeline_displayed_gb_;
    QRadioButton* source_rb_;
    QRadioButton* preprocessed_rb_;

    ColorSelectorWidget* lout_selector_;
    ColorSelectorWidget* lin_selector_;

    QCheckBox* flip_cb_;
    QCheckBox* smooth_cb_;

    QCheckBox* display_overlay_cb_;

    DisplayConfig config_;
    Session session_;
};

} // namespace ofeli_app

#endif // DISPLAY_SETTINGS_WIDGET_HPP
