// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ui_appearance.hpp"

#include <QWidget>

class QHBoxLayout;
class QToolButton;

namespace fluvel
{

class VolumeSlider;
class StyledToolButton;

/**
 * @brief Compact widget used to control audio volume.
 *
 * The widget combines a volume button and a horizontal slider.
 * It supports mute/unmute, direct volume changes and optional
 * modern or native rendering.
 */
class VolumeControlWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a volume control widget.
     *
     * @param parent Parent widget.
     * @param appearance Slider appearance.
     */
    explicit VolumeControlWidget(QWidget* parent = nullptr,
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
     * @brief Returns the volume button.
     */
    QToolButton* button() const;

    /**
     * @brief Returns the slider.
     */
    VolumeSlider* slider() const;

signals:
    /**
     * @brief Emitted when the volume changes.
     */
    void volumeChanged(int volume);

    /**
     * @brief Emitted when mute/unmute is requested.
     */
    void muteRequested();

private slots:

    void onButtonClicked();

    void onSliderChanged(int value);

private:
    void toggleSlider();

    void updateIcon();

    StyledToolButton* button_{nullptr};

    VolumeSlider* slider_{nullptr};

    int lastNonZeroVolume_{50};
};

} // namespace fluvel