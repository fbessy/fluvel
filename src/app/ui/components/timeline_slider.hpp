// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "jump_slider.hpp"

#include <QPainter>

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Slider used to navigate within a media timeline.
 *
 * The slider supports direct seeking by clicking anywhere on the
 * timeline and can optionally use a custom fullscreen appearance.
 *
 * When hovering the slider, the corresponding media position is
 * displayed near the cursor.
 */
class TimelineSlider : public JumpSlider
{
public:
    /**
     * @brief Constructs a timeline slider.
     *
     * @param parent Parent widget.
     * @param fullscreenStyle Enables the custom fullscreen rendering style.
     */
    explicit TimelineSlider(QWidget* parent = nullptr, bool fullscreenStyle = false);

    /**
     * @brief Returns whether the custom fullscreen style is enabled.
     *
     * @return True if the slider uses the fullscreen appearance.
     */
    bool isFullscreenStyle() const;

protected:
    /**
     * @brief Handles mouse move events.
     *
     * Displays the media position corresponding to the current mouse location.
     *
     * @param event Mouse event.
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse leave events.
     *
     * Clears hover feedback when the cursor leaves the slider
     * @param event Leave event.
     */
    void leaveEvent(QEvent* event) override;

    /**
     * @brief Paints the slider.
     *
     * Uses the default Qt rendering when fullscreen style is disabled.
     * Otherwise draws the custom Fluvel fullscreen appearance.
     *
     * @param event Paint event.
     */
    void paintEvent(QPaintEvent* event) override;

private:
    bool fullscreenStyle_{false};

    QString hoverText_;

    bool showHoverTime_{false};

    QPointF hoverPosition_;
};

} // namespace fluvel