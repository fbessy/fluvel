#include "display_settings_widget.hpp"
#include "application_settings.hpp"
#include "color_selector_widget.hpp"
#include "color_adapters.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QColorDialog>

#include <cassert>

namespace ofeli_app
{

DisplaySettingsWidget::DisplaySettingsWidget(QWidget* parent,
                                             Session session)
    : QWidget(parent), session_(session)
{
    if ( session_ == Session::Image )
        config_ = AppSettings::instance().imgConfig.display;
    else if ( session_ == Session::Camera )
        config_ = AppSettings::instance().camConfig.display;;


    lout_selector_ = new ColorSelectorWidget(this,
                                             toQColor(config_.l_out_color) );

    lin_selector_ = new ColorSelectorWidget(this,
                                            toQColor(config_.l_in_color) );

    QVBoxLayout* lout_layout = new QVBoxLayout;
    lout_layout->addWidget(lout_selector_);
    lout_layout->setContentsMargins(0, 0, 0, 0);
    lout_layout->setContentsMargins(0,0,0,0);

    QGroupBox* lout_gb = new QGroupBox(tr("Lout"));
    lout_gb->setLayout(lout_layout);
    lout_gb->setCheckable(true);
    lout_gb->setChecked( config_.l_out_displayed );

    lout_gb->setFlat(true);
    lout_gb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QVBoxLayout* lin_layout = new QVBoxLayout;
    lin_layout->addWidget(lin_selector_);
    lin_layout->setContentsMargins(0, 0, 0, 0);
    lin_layout->setContentsMargins(0,0,0,0);

    QGroupBox* lin_gb = new QGroupBox(tr("Lin"));
    lin_gb->setLayout(lin_layout);
    lin_gb->setCheckable(true);
    lin_gb->setChecked( config_.l_in_displayed );

    lin_gb->setFlat(true);
    lin_gb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    display_overlay_cb_ = new QCheckBox(tr("Algorithm overlay"));
    display_overlay_cb_->setChecked( config_.algorithm_overlay );

    input_displayed_cb_ = new QCheckBox(tr("Input image"));
    input_displayed_cb_->setChecked( config_.input_displayed );

    flip_cb_ = new QCheckBox(tr("Flip horizontally"));
    flip_cb_->setChecked( config_.flip_horizontal );

    QVBoxLayout* widget_layout = new QVBoxLayout;
    widget_layout->addWidget(lout_gb);
    widget_layout->addWidget(lin_gb);
    widget_layout->addWidget(display_overlay_cb_);
    widget_layout->addWidget(input_displayed_cb_);

    if ( session_ == Session::Camera )
        widget_layout->addWidget(flip_cb_);

    widget_layout->addStretch();

    setLayout( widget_layout );

    connect(lout_gb, &QGroupBox::toggled,
            this, [this](bool checked)
            {
                config_.l_out_displayed = checked;
                setConfig();
            });

    connect(lout_selector_, &ColorSelectorWidget::colorSelected,
            this, [this](const QColor& color)
            {
                config_.l_out_color = toRgb_uc(color);
                setConfig();
            });

    connect(lin_gb, &QGroupBox::toggled,
            this, [this](bool checked)
            {
                config_.l_in_displayed = checked;
                setConfig();
            });

    connect(lin_selector_, &ColorSelectorWidget::colorSelected,
            this, [this](const QColor& color)
            {
                config_.l_in_color = toRgb_uc(color);
                setConfig();
            });

    connect(display_overlay_cb_, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                config_.algorithm_overlay = checked;
                setConfig();
            });

    connect(input_displayed_cb_, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                config_.input_displayed = checked;
                setConfig();
            });

    if ( session_ == Session::Camera )
    {
        connect(flip_cb_, &QCheckBox::toggled,
                this, [this](bool checked)
                {
                    config_.flip_horizontal = checked;
                    setConfig();
                });
    }

    refresh_input_displayed_cb_availability();

    connect(&AppSettings::instance(), &ApplicationSettings::imgSettingsChanged,
            this,                     &DisplaySettingsWidget::onImgSettingsChanged);

    connect(&AppSettings::instance(), &ApplicationSettings::videoSettingsChanged,
            this,                     &DisplaySettingsWidget::onVideoSettingsChanged);
}

void DisplaySettingsWidget::setConfig()
{
    if ( session_ == Session::Image )
        AppSettings::instance().set_img_display_config(config_);
    else if ( session_ == Session::Camera )
        AppSettings::instance().set_cam_display_config(config_);
}

void DisplaySettingsWidget::refresh_input_displayed_cb_availability()
{
    bool isEnabled = false;

    if ( session_ == Session::Image )
    {
        const bool hasDownscale = AppSettings::instance().imgConfig.compute.downscale.hasDownscale;
        const auto& fc = AppSettings::instance().imgConfig.compute.processing;

        if ( hasDownscale || fc.hasProcessing() )
        {
            isEnabled = true;
        }
    }
    else if ( session_ == Session::Camera )
    {
        const bool hasDownscale = AppSettings::instance().camConfig.compute.downscale.hasDownscale;
        const bool has_filter = AppSettings::instance().camConfig.compute.hasTemporalFiltering;

        isEnabled = ( hasDownscale || has_filter );
    }

    if ( !isEnabled )
        input_displayed_cb_->setChecked(false);


    input_displayed_cb_->setEnabled( isEnabled );
}

void DisplaySettingsWidget::onImgSettingsChanged()
{
    refresh_input_displayed_cb_availability();
}

void DisplaySettingsWidget::onVideoSettingsChanged()
{
    refresh_input_displayed_cb_availability();
}

}
