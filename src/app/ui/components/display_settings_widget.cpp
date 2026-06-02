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

namespace fluvel
{

DisplaySettingsWidget::DisplaySettingsWidget(const DisplayConfig& config, QWidget* parent)
    : QWidget(parent)
    , config_(config)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    displayModeGroupBox_ = new QGroupBox(tr("Image:"));
    sourceRadioButton_ = new QRadioButton(tr("Source"));
    preprocessedRadioButton_ = new QRadioButton(tr("Preprocessed"));
    sourceRadioButton_->setChecked(config_.displayMode == ImageDisplayMode::Source);
    preprocessedRadioButton_->setChecked(config_.displayMode == ImageDisplayMode::Preprocessed);

    QVBoxLayout* displayModeLayout = new QVBoxLayout;
    displayModeLayout->addWidget(sourceRadioButton_);
    displayModeLayout->addWidget(preprocessedRadioButton_);
    displayModeGroupBox_->setLayout(displayModeLayout);

    outerContourColorSelector_ = new ColorSelectorWidget(this, toQColor(config_.outerContourColor));

    innerContourColorSelector_ = new ColorSelectorWidget(this, toQColor(config_.innerContourColor));

    QVBoxLayout* outerLayout = new QVBoxLayout;
    outerLayout->addWidget(outerContourColorSelector_);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* outerColorGroupBox = new QGroupBox(tr("Outer Contour"));
    outerColorGroupBox->setLayout(outerLayout);
    outerColorGroupBox->setCheckable(true);
    outerColorGroupBox->setChecked(config_.outerContourVisible);

    outerColorGroupBox->setFlat(true);
    outerColorGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QVBoxLayout* innerLayout = new QVBoxLayout;
    innerLayout->addWidget(innerContourColorSelector_);
    innerLayout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* innerColorGroupBox = new QGroupBox(tr("Inner Contour"));
    innerColorGroupBox->setLayout(innerLayout);
    innerColorGroupBox->setCheckable(true);
    innerColorGroupBox->setChecked(config_.innerContourVisible);

    innerColorGroupBox->setFlat(true);
    innerColorGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    mirrorModeCheckBox_ = new QCheckBox(tr("Selfie Mirror"));
    mirrorModeCheckBox_->setChecked(config_.mirrorMode);

    smoothDisplayCheckBox_ = new QCheckBox(tr("Smooth Display"));
    smoothDisplayCheckBox_->setChecked(config_.smoothDisplay);

    QGroupBox* renderingGroupBox = new QGroupBox(tr("Rendering:"));
    QVBoxLayout* renderingLayout = new QVBoxLayout;
    renderingLayout->addWidget(mirrorModeCheckBox_);
    renderingLayout->addWidget(smoothDisplayCheckBox_);
    renderingGroupBox->setLayout(renderingLayout);

    overlayCheckBox_ = new QCheckBox(tr("Overlay"));
    overlayCheckBox_->setChecked(config_.algorithmOverlayEnabled);

    miniMapCheckBox_ = new QCheckBox(tr("Mini-map"));
    miniMapCheckBox_->setChecked(config_.miniMapEnabled);

    auto* title = new QLabel(tr("View"));
    title->setStyleSheet("font-weight: bold; font-size: 14px;");

    QVBoxLayout* widgetLayout = new QVBoxLayout;
    widgetLayout->addWidget(title);
    widgetLayout->addSpacing(12);
    widgetLayout->addWidget(displayModeGroupBox_);
    widgetLayout->addWidget(outerColorGroupBox);
    widgetLayout->addWidget(innerColorGroupBox);
    widgetLayout->addWidget(renderingGroupBox);
    widgetLayout->addSpacing(6);
    widgetLayout->addWidget(overlayCheckBox_);
    widgetLayout->addWidget(miniMapCheckBox_);

    widgetLayout->addStretch();

    setLayout(widgetLayout);

    connect(outerColorGroupBox, &QGroupBox::toggled, this,
            [this](bool checked)
            {
                config_.outerContourVisible = checked;
                emit displayConfigChanged(config_);
            });

    connect(outerContourColorSelector_, &ColorSelectorWidget::colorSelected, this,
            [this](const QColor& color)
            {
                config_.outerContourColor = toRgb_uc(color);
                emit displayConfigChanged(config_);
            });

    connect(innerColorGroupBox, &QGroupBox::toggled, this,
            [this](bool checked)
            {
                config_.innerContourVisible = checked;
                emit displayConfigChanged(config_);
            });

    connect(innerContourColorSelector_, &ColorSelectorWidget::colorSelected, this,
            [this](const QColor& color)
            {
                config_.innerContourColor = toRgb_uc(color);
                emit displayConfigChanged(config_);
            });

    connect(overlayCheckBox_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.algorithmOverlayEnabled = checked;
                emit displayConfigChanged(config_);
            });

    connect(miniMapCheckBox_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.miniMapEnabled = checked;
                emit displayConfigChanged(config_);
            });

    connect(sourceRadioButton_, &QRadioButton::toggled, this,
            [this](bool checked)
            {
                if (!checked)
                    return;

                config_.displayMode = ImageDisplayMode::Source;
                emit displayConfigChanged(config_);
            });

    connect(preprocessedRadioButton_, &QRadioButton::toggled, this,
            [this](bool checked)
            {
                if (!checked)
                    return;

                config_.displayMode = ImageDisplayMode::Preprocessed;
                emit displayConfigChanged(config_);
            });

    connect(mirrorModeCheckBox_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.mirrorMode = checked;
                emit displayConfigChanged(config_);
            });

    connect(smoothDisplayCheckBox_, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                config_.smoothDisplay = checked;
                emit displayConfigChanged(config_);
            });
}

void DisplaySettingsWidget::updateDisplayModeAvailability(bool hasPreprocessing)
{
    displayModeGroupBox_->setEnabled(hasPreprocessing);

    if (!hasPreprocessing)
    {
        sourceRadioButton_->setChecked(true);
        preprocessedRadioButton_->setChecked(false);
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

} // namespace fluvel
