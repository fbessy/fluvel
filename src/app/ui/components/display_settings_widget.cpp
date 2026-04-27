// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "display_settings_widget.hpp"
#include "color_adapters.hpp"
#include "color_selector_widget.hpp"

#include <QColorDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QVBoxLayout>

#include <cassert>

namespace fluvel_app
{

DisplaySettingsWidget::DisplaySettingsWidget(const DisplayConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    pipeline_displayed_gb_ = new QGroupBox(tr("Image :"));
    source_rb_ = new QRadioButton(tr("Source"));
    preprocessed_rb_ = new QRadioButton(tr("Preprocessed"));
    source_rb_->setChecked(config_.displayMode == ImageDisplayMode::Source);
    preprocessed_rb_->setChecked(config_.displayMode == ImageDisplayMode::Preprocessed);

    QVBoxLayout* pipeline_layout = new QVBoxLayout;
    pipeline_layout->addWidget(source_rb_);
    pipeline_layout->addWidget(preprocessed_rb_);
    pipeline_displayed_gb_->setLayout(pipeline_layout);

    lout_selector_ = new ColorSelectorWidget(this, toQColor(config_.outerContourColor));

    lin_selector_ = new ColorSelectorWidget(this, toQColor(config_.innerContourColor));

    QVBoxLayout* lout_layout = new QVBoxLayout;
    lout_layout->addWidget(lout_selector_);
    lout_layout->setContentsMargins(0, 0, 0, 0);
    lout_layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* lout_gb = new QGroupBox(tr("Outer Contour"));
    lout_gb->setLayout(lout_layout);
    lout_gb->setCheckable(true);
    lout_gb->setChecked(config_.outerContourVisible);

    lout_gb->setFlat(true);
    lout_gb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QVBoxLayout* lin_layout = new QVBoxLayout;
    lin_layout->addWidget(lin_selector_);
    lin_layout->setContentsMargins(0, 0, 0, 0);
    lin_layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* lin_gb = new QGroupBox(tr("Inner Contour"));
    lin_gb->setLayout(lin_layout);
    lin_gb->setCheckable(true);
    lin_gb->setChecked(config_.innerContourVisible);

    lin_gb->setFlat(true);
    lin_gb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    flip_cb_ = new QCheckBox(tr("Selfie Mirror"));
    flip_cb_->setChecked(config_.mirrorMode);

    smooth_cb_ = new QCheckBox(tr("Smooth Display"));
    smooth_cb_->setChecked(config_.smoothDisplay);

    QGroupBox* rendering_gb = new QGroupBox(tr("Rendering:"));
    QVBoxLayout* rendering_layout = new QVBoxLayout;
    rendering_layout->addWidget(flip_cb_);
    rendering_layout->addWidget(smooth_cb_);
    rendering_gb->setLayout(rendering_layout);

    display_overlay_cb_ = new QCheckBox(tr("Overlay"));
    display_overlay_cb_->setChecked(config_.algorithmOverlayEnabled);

    auto* title = new QLabel(tr("View"));
    title->setStyleSheet("font-weight: bold; font-size: 14px;");

    QVBoxLayout* widget_layout = new QVBoxLayout;
    widget_layout->addWidget(title);
    widget_layout->addSpacing(12);
    widget_layout->addWidget(pipeline_displayed_gb_);
    widget_layout->addWidget(lout_gb);
    widget_layout->addWidget(lin_gb);
    widget_layout->addWidget(rendering_gb);
    widget_layout->addSpacing(6);
    widget_layout->addWidget(display_overlay_cb_);

    widget_layout->addStretch();

    setLayout(widget_layout);

    connect(lout_gb, &QGroupBox::toggled, this,
            [this](bool checked)
            {
                config_.outerContourVisible = checked;
                emit displayConfigChanged(config_);
            });

    connect(lout_selector_, &ColorSelectorWidget::colorSelected, this,
            [this](const QColor& color)
            {
                config_.outerContourColor = toRgb_uc(color);
                emit displayConfigChanged(config_);
            });

    connect(lin_gb, &QGroupBox::toggled, this,
            [this](bool checked)
            {
                config_.innerContourVisible = checked;
                emit displayConfigChanged(config_);
            });

    connect(lin_selector_, &ColorSelectorWidget::colorSelected, this,
            [this](const QColor& color)
            {
                config_.innerContourColor = toRgb_uc(color);
                emit displayConfigChanged(config_);
            });

    connect(display_overlay_cb_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.algorithmOverlayEnabled = checked;
                emit displayConfigChanged(config_);
            });

    connect(source_rb_, &QRadioButton::toggled, this,
            [this](bool checked)
            {
                if (!checked)
                    return;

                config_.displayMode = ImageDisplayMode::Source;
                emit displayConfigChanged(config_);
            });

    connect(preprocessed_rb_, &QRadioButton::toggled, this,
            [this](bool checked)
            {
                if (!checked)
                    return;

                config_.displayMode = ImageDisplayMode::Preprocessed;
                emit displayConfigChanged(config_);
            });

    connect(flip_cb_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.mirrorMode = checked;
                emit displayConfigChanged(config_);
            });

    connect(smooth_cb_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.smoothDisplay = checked;
                emit displayConfigChanged(config_);
            });
}

void DisplaySettingsWidget::updatePipelineAvailability(bool hasPreprocessing)
{
    pipeline_displayed_gb_->setEnabled(hasPreprocessing);

    if (!hasPreprocessing)
    {
        source_rb_->setChecked(true);
        preprocessed_rb_->setChecked(false);
    }
}

void DisplaySettingsWidget::setPanelVisible(bool visible)
{
    if (isAnimating_)
        return;

    const bool open = (maximumWidth() > 0);

    if (visible == open)
        return;

    animate(visible);
}

void DisplaySettingsWidget::animate(bool open)
{
    isAnimating_ = true;

    // On s'assure que le widget est visible avant de calculer
    if (open)
        show();

    // Calcul dynamique basé sur le layout
    adjustSize();
    const int targetWidth = minimumSizeHint().width();

    const int start = open ? 0 : width();
    const int end = open ? targetWidth : 0;

    auto* anim = new QPropertyAnimation(this, "maximumWidth");
    anim->setDuration(200);
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setEasingCurve(open ? QEasingCurve::OutCubic : QEasingCurve::InCubic);

    connect(anim, &QPropertyAnimation::finished, this,
            [this, open]()
            {
                if (!open)
                    hide();

                isAnimating_ = false;
            });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace fluvel_app
