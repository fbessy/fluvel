// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QWheelEvent>
#include <QtCore/Qt>

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

class QMouseEvent;

namespace fluvel_app
{

class ImageViewerWidget;

/**
 * @brief Base class for modular interaction behaviors.
 *
 * A behavior represents a single, reusable interaction unit
 * (e.g. pan, zoom, color picking, overlay display).
 *
 * Behaviors are typically managed by an InteractionSet, which
 * dispatches events to them.
 *
 * Event handling:
 * - Each handler returns true if the event is handled
 * - Returning false allows other behaviors to process the event
 *
 * Capture model:
 * - A behavior may enter a capturing state (isCapturing() == true)
 * - While capturing, it has priority over other behaviors
 *
 * Cursor management:
 * - activeCursor() is used when the behavior is capturing
 * - availableCursor() is used when the behavior is available but not active
 *
 * Priority:
 * - Behaviors can define a priority to influence dispatch order
 *
 * @note Behaviors are usually stateful and managed externally.
 */
class ImageViewerBehavior
{
public:
    virtual ~ImageViewerBehavior() = default;

    /**
     * @name Event handlers
     * @brief Input event callbacks. Return true if the event is handled.
     * @{
     */

    virtual bool wheel(ImageViewerWidget&, QWheelEvent*)
    {
        return false;
    }
    virtual bool mousePress(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseMove(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseRelease(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseDoubleClick(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }

    virtual bool dragEnter(ImageViewerWidget&, QDragEnterEvent*)
    {
        return false;
    }

    virtual bool dragMove(ImageViewerWidget&, QDragMoveEvent*)
    {
        return false;
    }

    virtual bool dragLeave(ImageViewerWidget&, QDragLeaveEvent*)
    {
        return false;
    }

    virtual bool drop(ImageViewerWidget&, QDropEvent*)
    {
        return false;
    }

    /** @} */

    /**
     * @name Interaction state
     * @brief Capture, cursor and priority control.
     * @{
     */

    /**
     * @brief Returns whether the behavior is currently capturing input.
     *      * When true, this behavior has priority over others.
     */
    virtual bool isCapturing() const
    {
        return false;
    }

    /**
     * @brief Cursor used while the behavior is active (capturing).
     */
    virtual Qt::CursorShape activeCursor() const
    {
        return Qt::ArrowCursor;
    }

    /**
     * @brief Cursor used when the behavior is available but not active.
     *      * @param hasImage True if an image is loaded.
     * @param isPanRelevant True if panning is meaningful.
     */
    virtual Qt::CursorShape availableCursor(bool /*hasImage*/, bool /*isPanRelevant*/,
                                            const ImageViewerWidget&, const QMouseEvent*) const
    {
        return Qt::ArrowCursor;
    }

    /**
     * @brief Returns the priority of the behavior.
     *      * Higher priority behaviors are processed first.
     */
    virtual int priority() const
    {
        return 0;
    }

    /**
     * @brief Cancels the current interaction state.
     *      * Called to reset the behavior (e.g. when interaction is interrupted).
     */
    virtual void cancel() {};

    /** @} */
};

} // namespace fluvel_app
