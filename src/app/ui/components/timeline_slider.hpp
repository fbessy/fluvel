// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "jump_slider.hpp"

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Slider used to navigate within a media timeline.
 *
 * This slider allows direct seeking by clicking or dragging at any position
 * on the timeline, updating its value according to the mouse position.
 */
class TimelineSlider : public JumpSlider
{
public:
    /**
     * @brief Constructs a timeline slider.
     *
     * @param parent Parent widget.
     */
    explicit TimelineSlider(QWidget* parent = nullptr);

protected:
    /**
     * @brief Handles mouse move events.
     *
     * Displays the media position corresponding to the current mouse location.
     *
     * @param event Mouse event.
     */
    void mouseMoveEvent(QMouseEvent* event) override;
};

} // namespace fluvel