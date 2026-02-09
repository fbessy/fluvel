#include "display_settings_widget.hpp"
#include "application_settings.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QColorDialog>

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

    int lout_index = get_index( config.l_out_color );

    if ( lout_index == ComboBoxColorIndex::SELECTED )
    {
        QPixmap pm(12,12);
        pm.fill( get_QRgb(config.l_out_color) );
        lout_color_cb_->setItemIcon(ComboBoxColorIndex::SELECTED,pm);
    }

    lout_color_cb_->setCurrentIndex( lout_index );

    lout_select_color_ = new QPushButton(tr("select"));

    QHBoxLayout* lout_layout = new QHBoxLayout;
    lout_layout->addWidget(lout_color_cb_);
    lout_layout->addWidget(lout_select_color_);

    QGroupBox* lout_gb = new QGroupBox(tr("Lout"));
    lout_gb->setLayout(lout_layout);
    lout_gb->setCheckable(true);
    lout_gb->setFlat(true);
    lout_gb->setChecked( config.l_out_displayed );


    lin_color_cb_ = new QComboBox;
    init_combobox_color( lin_color_cb_ );

    int lin_index = get_index( config.l_in_color );

    if ( lin_index == ComboBoxColorIndex::SELECTED )
    {
        QPixmap pm(12,12);
        pm.fill( get_QRgb(config.l_in_color) );
        lin_color_cb_->setItemIcon(ComboBoxColorIndex::SELECTED,pm);
    }

    lin_color_cb_->setCurrentIndex( lin_index );

    lin_select_color_ = new QPushButton(tr("select"));

    QHBoxLayout* lin_layout = new QHBoxLayout;
    lin_layout->addWidget(lin_color_cb_);
    lin_layout->addWidget(lin_select_color_);

    QGroupBox* lin_gb = new QGroupBox(tr("Lin"));
    lin_gb->setLayout(lin_layout);
    lin_gb->setCheckable(true);
    lin_gb->setFlat(true);
    lin_gb->setChecked( config.l_in_displayed );

    input_displayed_cb_ = new QCheckBox(tr("Input image"));
    input_displayed_cb_->setChecked( config.input_displayed );

    display_overlay_cb_ = new QCheckBox(tr("Algorithm overlay"));
    display_overlay_cb_->setChecked( config.algorithm_overlay );

    QVBoxLayout* right_layout = new QVBoxLayout;
    right_layout->addWidget(input_displayed_cb_);
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
                config_.l_out_displayed = checked;
                setConfig();
            });

    connect(lin_gb, &QGroupBox::toggled,
            this, [this](bool checked)
            {
                config_.l_in_displayed = checked;
                setConfig();
            });

    connect(input_displayed_cb_, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                config_.input_displayed = checked;
                setConfig();
            });

    connect(display_overlay_cb_, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                config_.algorithm_overlay = checked;
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

    connect(lout_select_color_,  &QPushButton::clicked,
            this,                &DisplaySettingsWidget::set_color_out);

    connect(lin_select_color_,  &QPushButton::clicked,
            this,               &DisplaySettingsWidget::set_color_in);

    refresh_input_displayed_cb_availability();

    connect(&AppSettings::instance(), &ApplicationSettings::imgSettingsApplied,
            this,                     &DisplaySettingsWidget::onImgSettingsApplied);

    connect(&AppSettings::instance(), &ApplicationSettings::camSettingsApplied,
            this,                     &DisplaySettingsWidget::onCamSettingsApplied);
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

void DisplaySettingsWidget::set_color_out()
{
    // Sélection d'une QColor a partir d'une boite de dialogue couleur
    QColor color_out = QColorDialog::getColor(Qt::white, this, tr("Select Lout color"));
    if( color_out.isValid() )
    {
        QPixmap pm(12,12);
        pm.fill(color_out);

        lout_color_cb_->blockSignals(true);

        lout_color_cb_->setItemIcon(ComboBoxColorIndex::SELECTED,pm);
        lout_color_cb_->setCurrentIndex(ComboBoxColorIndex::SELECTED);

        lout_color_cb_->blockSignals(false);

        config_.l_out_color.red   = (unsigned char)(color_out.red());
        config_.l_out_color.green = (unsigned char)(color_out.green());
        config_.l_out_color.blue  = (unsigned char)(color_out.blue());
        setConfig();
    }
}

void DisplaySettingsWidget::set_color_in()
{
    // Selection d'une QColor à partir d'une boîte de dialogue couleur
    QColor color_in = QColorDialog::getColor(Qt::white, this, tr("Select Lin color"));
    if( color_in.isValid() )
    {
        QPixmap pm(12,12);
        pm.fill(color_in);

        lin_color_cb_->blockSignals(true);

        lin_color_cb_->setItemIcon(ComboBoxColorIndex::SELECTED,pm);
        lin_color_cb_->setCurrentIndex(ComboBoxColorIndex::SELECTED);

        lin_color_cb_->blockSignals(false);


        config_.l_in_color.red   = (unsigned char)(color_in.red());
        config_.l_in_color.green = (unsigned char)(color_in.green());
        config_.l_in_color.blue  = (unsigned char)(color_in.blue());
        setConfig();
    }
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
        const bool has_downscale = AppSettings::instance().imgSessSettings.downscale_conf.has_downscale;
        const bool has_preprocess = AppSettings::instance().imgSessSettings.has_preprocess;
        const auto& fc = AppSettings::instance().imgSessSettings.filtering_conf;

        if (      has_downscale
            || ( has_preprocess && (    fc.has_gaussian_noise
                                   || fc.has_salt_noise
                                   || fc.has_speckle_noise
                                   || fc.has_mean_filt
                                   || fc.has_gaussian_filt
                                   || fc.has_median_filt
                                   || fc.has_aniso_diff
                                   || fc.has_open_filt
                                   || fc.has_close_filt
                                   || fc.has_top_hat_filt ) ) )
        {
            isEnabled = true;
        }
    }
    else if ( session_ == Session::Camera )
    {
        const bool has_downscale = AppSettings::instance().camSessSettings.downscale_conf.has_downscale;
        const bool has_filter = AppSettings::instance().camSessSettings.has_temporal_filtering;

        isEnabled = ( has_downscale || has_filter );
    }

    if ( !isEnabled )
        input_displayed_cb_->setChecked(false);


    input_displayed_cb_->setEnabled( isEnabled );
}

void DisplaySettingsWidget::onImgSettingsApplied()
{
    refresh_input_displayed_cb_availability();
}

void DisplaySettingsWidget::onCamSettingsApplied()
{
    refresh_input_displayed_cb_availability();
}

}
