// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"
#include "image_viewer_widget.hpp"

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Behavior toggling fullscreen mode on double-click.
 *
 * This behavior listens for mouse double-click events and switches
 * the associated ImageViewerWidget between normal and fullscreen modes.
 *
 * @note The event is typically handled only for left-button double-click.
 */
class FullscreenBehavior : public ImageViewerBehavior
{
public:
    /**
     * @brief Handles mouse double-click events.
     *
     * Toggles fullscreen mode.
     */
    bool mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Returns the behavior priority.
     *
     * Higher-priority behaviors are evaluated before lower-priority
     * ones when processing input events.
     *
     * @return Priority value.
     */
    int priority() const override
    {
        return 80;
    }

    /**
     * @brief Returns the cursor shape associated with fullscreen mode.
     *
     * The cursor is hidden while the viewer is fullscreen and the user
     * is inactive.
     */
    Qt::CursorShape availableCursor(bool, bool, const ImageViewerWidget& view,
                                    const QMouseEvent*) const override;
};

} // namespace fluvel
