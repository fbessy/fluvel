#ifndef DISPLAY_SETTINGS_WIDGET_HPP
#define DISPLAY_SETTINGS_WIDGET_HPP

#include "common_settings.hpp"

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

namespace ofeli_app
{

class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    DisplaySettingsWidget(QWidget* parent,
                          const DisplayConfig& config,
                          Session session);

private slots:
    void set_color_out();
    void set_color_in();

private:

    void init_combobox_color(QComboBox* color_cb);
    void setConfig();

    QCheckBox* input_displayed_cb_;

    QComboBox* lout_color_cb_;
    QPushButton* lout_select_color_;

    QComboBox* lin_color_cb_;
    QPushButton* lin_select_color_;

    QCheckBox* display_overlay_cb_;

    DisplayConfig config_;
    Session session_;
};

}

#endif // DISPLAY_SETTINGS_WIDGET_HPP
