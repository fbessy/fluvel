// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "volume_control_widget.hpp"

#include "icon_loader.hpp"
#include "styled_tool_button.hpp"
#include "volume_slider.hpp"

#include <QHBoxLayout>
#include <QToolButton>

namespace fluvel
{

VolumeControlWidget::VolumeControlWidget(QWidget* parent, ui::Appearance appearance)
    : QWidget(parent)
{
    button_ = new StyledToolButton(this, appearance);

    slider_ = new VolumeSlider(this, appearance);

    slider_->setRange(0, 100);
    slider_->setValue(50);
    slider_->setFixedWidth(110);
    slider_->hide();

    auto* layout = new QHBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    layout->addWidget(button_);
    layout->addWidget(slider_);

    connect(button_, &QToolButton::clicked, this, &VolumeControlWidget::onButtonClicked);

    connect(slider_, &QSlider::valueChanged, this, &VolumeControlWidget::onSliderChanged);

    updateIcon();
}

int VolumeControlWidget::volume() const
{
    return slider_->value();
}

void VolumeControlWidget::setVolume(int volume)
{
    slider_->setValue(volume);

    if (volume > 0)
        lastNonZeroVolume_ = volume;

    updateIcon();
}

QToolButton* VolumeControlWidget::button() const
{
    return button_;
}

VolumeSlider* VolumeControlWidget::slider() const
{
    return slider_;
}

void VolumeControlWidget::onButtonClicked()
{
    toggleSlider();
}

void VolumeControlWidget::toggleSlider()
{
    slider_->setVisible(!slider_->isVisible());
}

void VolumeControlWidget::onSliderChanged(int value)
{
    if (value > 0)
        lastNonZeroVolume_ = value;

    updateIcon();

    emit volumeChanged(value);
}

void VolumeControlWidget::updateIcon()
{
    QIcon icon;

    if (slider_->value() == 0)
    {
        icon = il::loadIcon(QIcon::ThemeIcon::AudioVolumeMuted,
                            ":/icons/status/audio-volume-muted.svg", il::IconMode::Light);
    }
    else if (slider_->value() < 33)
    {
        icon = il::loadIcon(QIcon::ThemeIcon::AudioVolumeLow, ":/icons/status/audio-volume-low.svg",
                            il::IconMode::Light);
    }
    else if (slider_->value() < 66)
    {
        icon = il::loadIcon(QIcon::ThemeIcon::AudioVolumeMedium,
                            ":/icons/status/audio-volume-medium.svg", il::IconMode::Light);
    }
    else
    {
        icon = il::loadIcon(QIcon::ThemeIcon::AudioVolumeHigh,
                            ":/icons/status/audio-volume-high.svg", il::IconMode::Light);
    }

    button_->setIcon(icon);
}

} // namespace fluvel