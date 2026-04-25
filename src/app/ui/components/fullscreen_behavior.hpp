// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"

class QMouseEvent;

namespace fluvel_app
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
     *      * Toggles fullscreen mode.
     */
    bool mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event) override;
};

} // namespace fluvel_app
