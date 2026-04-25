// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"
#include "image_viewer_behavior.hpp"

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

/**
 * @brief Behavior handling drag-and-drop of images.
 *
 * This behavior enables drag-and-drop interactions on the viewer,
 * allowing users to load images by dropping files.
 *
 * It manages drag enter/move/leave feedback and processes the drop event.
 *
 * Priority:
 * - High priority to ensure drag-and-drop interactions are handled
 *   before other behaviors.
 */
class DragDropBehavior : public ImageViewerBehavior
{
public:
    /**
     * @name Drag & drop events
     * @brief Handle drag-and-drop interactions.
     * @{
     */
    bool dragEnter(ImageViewerWidget&, QDragEnterEvent*) override;
    bool dragMove(ImageViewerWidget&, QDragMoveEvent*) override;
    bool dragLeave(ImageViewerWidget&, QDragLeaveEvent*) override;
    bool drop(ImageViewerWidget&, QDropEvent*) override;
    /** @} */

    /**
     * @brief Returns the behavior priority.
     *      * A high priority ensures drag-and-drop is handled before
     * other interactions.
     */
    int priority() const override
    {
        return 100;
    }
};

} // namespace fluvel_app
