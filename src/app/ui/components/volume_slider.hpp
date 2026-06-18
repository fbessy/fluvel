// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "jump_slider.hpp"

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Slider used to control audio volume.
 *
 * This slider supports direct positioning by clicking anywhere on the
 * slider groove and displays the corresponding volume percentage while
 * hovering over the control.
 */
class VolumeSlider : public JumpSlider
{
public:
    /**
     * @brief Constructs a volume slider.
     *
     * @param parent Parent widget.
     */
    explicit VolumeSlider(QWidget* parent = nullptr);

protected:
    /**
     * @brief Handles mouse move events.
     *
     * Displays the volume percentage corresponding to the current
     * mouse position.
     *
     * @param event Mouse event.
     */
    void mouseMoveEvent(QMouseEvent* event) override;
};

} // namespace fluvel