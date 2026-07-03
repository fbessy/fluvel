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
 * @brief Provides a volume button and slider.
 *
 * VolumeController owns a volume button and a slider, emits user
 * interaction requests and updates their visual state.
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
    [[nodiscard]]
    int volume() const;

    /**
     * @brief Updates the displayed volume.
     *
     * This function updates the widget state without emitting
     * volumeRequested().
     *
     * @param volume Volume in the range [0, 100].
     */
    void setVolume(int volume);

    /**
     * @brief Updates the displayed muted state.
     *
     * This function updates the mute button appearance without emitting
     * toggleMuteRequested().
     *
     * @param muted @c true to display the muted state, @c false otherwise.
     */
    void setMuted(bool muted);

    /**
     * @brief Enables or disables the volume controls.
     *
     * Both the volume button and the slider are enabled or disabled
     * simultaneously.
     *
     * @param enabled True to enable the controls, false to disable them.
     */
    void setControlsEnabled(bool enabled);

    /**
     * @brief Shows or hides the volume controls.
     *
     * Both the volume button and the slider are shown or hidden
     * simultaneously.
     *
     * @param visible True to show the controls, false to hide them.
     */
    void setControlsVisible(bool visible);

    /**
     * @brief Returns the volume button.
     */
    [[nodiscard]]
    StyledToolButton* button() const;

    /**
     * @brief Returns the slider.
     */
    [[nodiscard]]
    VolumeSlider* slider() const;

signals:
    /**
     * @brief Emitted when the user requests a volume change.
     *
     * @param volume Requested volume in the range [0, 100].
     */
    void volumeRequested(int volume);

    /**
     * @brief Emitted when the user requests to toggle the muted state.
     */
    void toggleMuteRequested();

private:
    void updateIcon() const;

    StyledToolButton* button_{nullptr};
    VolumeSlider* slider_{nullptr};

    bool muted_{false};
};

} // namespace fluvel