// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QMouseEvent>
#include <QWheelEvent>

#include <QtCore/Qt>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

class ImageViewerWidget;

/**
 * @brief Base interface for image viewer interaction handling.
 *
 * This interface defines how input events (mouse, wheel, drag & drop)
 * are processed by an ImageViewerWidget.
 *
 * Implementations can override only the relevant methods. By default,
 * all event handlers return false (event not handled).
 *
 * Event handling:
 * - Returning true means the event is handled
 * - Returning false allows further propagation
 *
 * Cursor handling:
 * - cursorForEvent() provides the cursor shape depending on the
 *   current interaction state
 *
 * @note This interface does not enforce any ownership or lifetime.
 *       Multiple interaction layers may exist (e.g. InteractionSet).
 *       Implementations should remain stateless or explicitly manage state.
 */
class ImageViewerInteraction
{
public:
    virtual ~ImageViewerInteraction() = default;

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
     * @brief Cursor and lifecycle control.
     * @{
     */
    virtual Qt::CursorShape cursorForEvent(const ImageViewerWidget& /*view*/, bool /*hasImage*/,
                                           bool /*isPanRelevant*/,
                                           const QMouseEvent* /*event*/) const
    {
        return Qt::ArrowCursor;
    }

    virtual void cancel()
    {
    }
    /** @} */
};

} // namespace fluvel_app
