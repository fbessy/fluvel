// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "volume_controller.hpp"

#include "icon_loader.hpp"
#include "styled_tool_button.hpp"
#include "volume_slider.hpp"

#include <QToolButton>

namespace fluvel
{

VolumeController::VolumeController(QWidget* parent, ui::Appearance appearance)
    : QObject(parent)
{
    button_ = new StyledToolButton(parent, appearance);

    slider_ = new VolumeSlider(parent, appearance);

    slider_->setRange(0, 100);
    slider_->setValue(50);

    connect(button_, &QToolButton::clicked, this, &VolumeController::onMuteButtonClicked);

    connect(slider_, &QSlider::valueChanged, this, &VolumeController::onSliderChanged);

    updateIcon();
}

int VolumeController::volume() const
{
    return slider_->value();
}

void VolumeController::setVolume(int volume)
{
    slider_->setValue(volume);

    if (volume > 0)
        lastNonZeroVolume_ = volume;

    updateIcon();
}

StyledToolButton* VolumeController::button() const
{
    return button_;
}

VolumeSlider* VolumeController::slider() const
{
    return slider_;
}

void VolumeController::onMuteButtonClicked()
{
    if (slider_->value() == 0)
    {
        slider_->setValue(lastNonZeroVolume_);
    }
    else
    {
        lastNonZeroVolume_ = slider_->value();
        slider_->setValue(0);
    }
}

void VolumeController::onSliderChanged(int value)
{
    if (value > 0)
        lastNonZeroVolume_ = value;

    updateIcon();

    emit volumeChanged(value);
}

void VolumeController::updateIcon()
{
    QIcon icon;

    if (slider_->value() == 0)
    {
        icon = il::loadIcon(":/icons/status/audio-volume-muted.svg", il::IconMode::Light);
    }
    else if (slider_->value() < 33)
    {
        icon = il::loadIcon(":/icons/status/audio-volume-low.svg", il::IconMode::Light);
    }
    else if (slider_->value() < 66)
    {
        icon = il::loadIcon(":/icons/status/audio-volume-medium.svg", il::IconMode::Light);
    }
    else
    {
        icon = il::loadIcon(":/icons/status/audio-volume-high.svg", il::IconMode::Light);
    }

    button_->setIcon(icon);
}

void VolumeController::setControlsEnabled(bool enabled)
{
    button_->setEnabled(enabled);
    slider_->setEnabled(enabled);
}

} // namespace fluvel