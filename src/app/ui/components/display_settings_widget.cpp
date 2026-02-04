#include "display_settings_widget.hpp"
#include "application_settings.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

#include <cassert>

namespace ofeli_app
{

DisplaySettingsWidget::DisplaySettingsWidget(QWidget* parent,
                                             const DisplayConfig& config,
                                             Session session)
    : QWidget(parent), config_(config), session_(session)
{
    lout_color_cb_ = new QComboBox;
    init_combobox_color( lout_color_cb_ );
    lout_select_color_ = new QPushButton(tr("select"));

    QHBoxLayout* lout_layout = new QHBoxLayout;
    lout_layout->addWidget(lout_color_cb_);
    lout_layout->addWidget(lout_select_color_);

    QGroupBox* lout_gb = new QGroupBox(tr("Lout"));
    lout_gb->setLayout(lout_layout);
    lout_gb->setCheckable(true);
    lout_gb->setFlat(true);


    lin_color_cb_ = new QComboBox;
    init_combobox_color( lin_color_cb_ );
    lin_select_color_ = new QPushButton(tr("select"));

    QHBoxLayout* lin_layout = new QHBoxLayout;
    lin_layout->addWidget(lin_color_cb_);
    lin_layout->addWidget(lin_select_color_);

    QGroupBox* lin_gb = new QGroupBox(tr("Lin"));
    lin_gb->setLayout(lin_layout);
    lin_gb->setCheckable(true);
    lin_gb->setFlat(true);

    preprocess_cb_ = new QCheckBox(tr("preprocess"));
    display_overlay_cb_ = new QCheckBox(tr("overlay"));

    QVBoxLayout* right_layout = new QVBoxLayout;
    right_layout->addWidget(preprocess_cb_);
    right_layout->addWidget(display_overlay_cb_);


    QHBoxLayout* widget_layout = new QHBoxLayout;
    widget_layout->addWidget(lout_gb);
    widget_layout->addWidget(lin_gb);
    widget_layout->addLayout(right_layout);

    widget_layout->setContentsMargins(8, 4, 8, 4);
    widget_layout->setSpacing(6);
    widget_layout->addStretch();

    setLayout( widget_layout );



    connect(lout_gb, &QGroupBox::toggled,
            this, [this](bool checked)
            {
                config_.display_l_out = checked;
                setConfig();
            });

    connect(lin_gb, &QGroupBox::toggled,
            this, [this](bool checked)
            {
                config_.display_l_in = checked;
                setConfig();
            });

    connect(preprocess_cb_, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                config_.display_preprocess = checked;
                setConfig();
            });

    connect(display_overlay_cb_, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                config_.display_overlay = checked;
                setConfig();
            });

    connect(lout_color_cb_, &QComboBox::currentIndexChanged,
            this, [this](int index)
            {
                config_.l_out_color = get_color(index);
                setConfig();
            });

    connect(lin_color_cb_, &QComboBox::currentIndexChanged,
            this, [this](int index)
            {
                config_.l_in_color = get_color(index);
                setConfig();
            });
}

void DisplaySettingsWidget::init_combobox_color(QComboBox* color_cb)
{
    assert( color_cb != nullptr );

    // QPixmap pm : petite image affichant la couleur devant le nom de la couleur dans le combobox
    QPixmap pm(12,12);

    pm.fill(Qt::red);
    color_cb->addItem (pm, tr("Red"));

    pm.fill(Qt::green);
    color_cb->addItem (pm, tr("Green"));

    pm.fill(Qt::blue);
    color_cb->addItem (pm, tr("Blue"));

    pm.fill(Qt::cyan);
    color_cb->addItem (pm, tr("Cyan"));

    pm.fill(Qt::magenta);
    color_cb->addItem (pm, tr("Magenta"));

    pm.fill(Qt::yellow);
    color_cb->addItem (pm, tr("Yellow"));

    pm.fill(Qt::black);
    color_cb->addItem (pm, tr("Black"));

    pm.fill(Qt::white);
    color_cb->addItem (pm, tr("White"));

    pm.fill(Qt::transparent);
    color_cb->addItem (pm,tr("Selected"));
}

void DisplaySettingsWidget::setConfig()
{
    if ( session_ == Session::Image )
        AppSettings::instance().set_img_display_config(config_);
    else if ( session_ == Session::Camera )
        AppSettings::instance().set_cam_display_config(config_);
}

}
