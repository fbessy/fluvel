// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "volume_controller.hpp"

#include "icon_loader.hpp"
#include "styled_tool_button.hpp"
#include "volume_slider.hpp"

#include <QMouseEvent>
#include <QToolButton>

#include <algorithm>

namespace fluvel
{

VolumeController::VolumeController(QWidget* parent, ui::Appearance appearance)
    : QObject(parent)
{
    button_ = new StyledToolButton(parent, appearance);
    button_->installEventFilter(this);

    slider_ = new VolumeSlider(parent, appearance);

    slider_->setRange(0, 100);
    slider_->setValue(50);

    updateIcon();

    connect(button_, &QToolButton::clicked, this,
            [this]()
            {
                emit toggleMuteRequested();
            });

    connect(slider_, &QSlider::valueChanged, this,
            [this](int volume)
            {
                emit volumeRequested(volume);
            });
}

int VolumeController::volume() const
{
    return slider_->value();
}

void VolumeController::setVolume(int volume)
{
    QSignalBlocker blocker(slider_);

    volume = std::clamp(volume, 0, 100);

    slider_->setValue(volume);

    updateIcon();
}

void VolumeController::setMuted(bool muted)
{
    if (muted_ == muted)
        return;

    muted_ = muted;

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

void VolumeController::setControlsEnabled(bool enabled)
{
    button_->setEnabled(enabled);
    slider_->setEnabled(enabled);
}

void VolumeController::setControlsVisible(bool visible)
{
    button_->setVisible(visible);
    slider_->setVisible(visible);
}

void VolumeController::updateIcon() const
{
    QIcon icon;

    const int volume = slider_->value();

    if (muted_ || volume == 0)
    {
        icon = il::loadIcon(":/icons/status/audio-volume-muted.svg", il::IconMode::Light);
    }
    else if (volume < 33)
    {
        icon = il::loadIcon(":/icons/status/audio-volume-low.svg", il::IconMode::Light);
    }
    else if (volume < 66)
    {
        icon = il::loadIcon(":/icons/status/audio-volume-medium.svg", il::IconMode::Light);
    }
    else
    {
        icon = il::loadIcon(":/icons/status/audio-volume-high.svg", il::IconMode::Light);
    }

    button_->setIcon(icon);
}

bool VolumeController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == button_ && event->type() == QEvent::MouseButtonRelease)
    {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::RightButton)
        {
            emit toggleMuteRequested();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

} // namespace fluvel