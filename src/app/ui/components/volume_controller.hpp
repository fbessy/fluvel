// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ui_appearance.hpp"

#include <QObject>

class QWidget;

namespace fluvel
{

class VolumeSlider;
class StyledToolButton;

/**
 * @brief Controls audio volume widgets.
 *
 * VolumeController owns a volume button and a slider and
 * synchronizes their state (mute, icon and current volume).
 *
 * Layout is intentionally left to the caller.
 */
class VolumeController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a volume control widget.
     *
     * @param parent Parent widget.
     * @param appearance Slider appearance.
     */
    explicit VolumeController(QWidget* parent = nullptr,
                              ui::Appearance appearance = ui::Appearance::Modern);

    /**
     * @brief Returns the current volume.
     */
    int volume() const;

    /**
     * @brief Sets the current volume.
     *
     * @param volume Volume in percent.
     */
    void setVolume(int volume);

    /**
     * @brief Show or hide widgets.
     *
     * @param visible Enable visibility.
     */
    void setControlsVisible(bool visible);

    /**
     * @brief Returns the volume button.
     */
    StyledToolButton* button() const;

    /**
     * @brief Returns the slider.
     */
    VolumeSlider* slider() const;

signals:
    /**
     * @brief Emitted when the volume changes.
     */
    void volumeChanged(int volume);

private:
    void onMuteButtonClicked();
    void onSliderChanged(int value);
    void updateIcon();

    StyledToolButton* button_{nullptr};
    VolumeSlider* slider_{nullptr};

    /// Volume restored after unmuting.
    int lastNonZeroVolume_{50};
};

} // namespace fluvel