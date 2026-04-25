// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"
#include "image_viewer_behavior.hpp"

#include <QPoint>

class QMouseEvent;

namespace fluvel_app
{

/**
 * @brief Behavior enabling panning of the image view.
 *
 * This behavior allows the user to move (pan) the view by dragging
 * with a mouse button (left button by default).
 *
 * Interaction flow:
 * - Activated on mouse press with the configured button
 * - While active, mouse movement translates the view
 * - Stops on mouse release or cancel()
 *
 * Characteristics:
 * - Captures interaction while active
 * - Updates the cursor to reflect interaction state
 * - Does not interfere with movable items in the scene
 *
 * The behavior is only available when panning is relevant and an image
 * is present in the viewer.
 */
class PanBehavior : public ImageViewerBehavior
{
public:
    /**
     * @brief Constructs the pan behavior.
     *      * @param button Mouse button used to trigger panning
     *               (default: Qt::LeftButton).
     */
    explicit PanBehavior(Qt::MouseButton button = Qt::LeftButton);

    /**
     * @brief Handles mouse press events.
     *      * Starts panning if the configured button is pressed.
     *      * @param view Associated image viewer.
     * @param event Mouse event.
     * @return true if the event is handled and capture begins.
     */
    bool mousePress(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse move events.
     *      * Translates the view according to mouse movement while capturing.
     *      * @param view Associated image viewer.
     * @param event Mouse event.
     * @return true if the event is handled.
     */
    bool mouseMove(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse release events.
     *      * Stops panning when the configured button is released.
     *      * @param view Associated image viewer.
     * @param event Mouse event.
     * @return true if the event is handled.
     */
    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Returns the cursor when the behavior is active.
     *      * @return Closed hand cursor during dragging.
     */
    Qt::CursorShape activeCursor() const override
    {
        return Qt::ClosedHandCursor;
    }

    /**
     * @brief Returns the cursor when the behavior is available.
     *      * Determines whether panning is possible at the current position and
     * adapts the cursor accordingly.
     *      * If the cursor is over a movable graphics item, panning is disabled
     * and the default cursor is used.
     *      * @param hasImage True if an image is loaded.
     * @param isPanRelevant True if panning is meaningful in current state.
     * @param view Associated image viewer.
     * @param e Optional mouse event (can be null).
     * @return Appropriate cursor shape.
     */
    Qt::CursorShape availableCursor(bool hasImage, bool isPanRelevant, const ImageViewerWidget& view,
                                    const QMouseEvent* e) const override
    {
        if (!hasImage || !isPanRelevant)
            return Qt::ArrowCursor;

        QPoint pos = e ? e->pos() : view.mapFromGlobal(QCursor::pos());

        const auto items = view.items(pos);

        for (auto* item : items)
        {
            if (item->flags() & QGraphicsItem::ItemIsMovable)
                return Qt::ArrowCursor;
        }

        return Qt::OpenHandCursor;
    }

    /**
     * @brief Returns whether the behavior is currently capturing input.
     */
    bool isCapturing() const override
    {
        return capturing_;
    }

    /**
     * @brief Cancels the behavior.
     *      * Stops any ongoing panning interaction.
     */
    void cancel() override
    {
        capturing_ = false;
    }

    /**
     * @brief Returns the priority of this behavior.
     *      * Higher priority behaviors are processed first.
     */
    int priority() const override
    {
        return 10;
    }

private:
    /// Mouse button used to trigger panning.
    Qt::MouseButton button_;

    /// True while the behavior is actively panning.
    bool capturing_{false};

    /// Last mouse position used to compute movement.
    QPoint lastPos_;
};

} // namespace fluvel_app
