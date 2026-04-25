// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_interaction.hpp"
#include "image_viewer_behavior.hpp"

#include <QtCore/Qt>

#include <memory>
#include <vector>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

/**
 * @brief Aggregates and dispatches multiple ImageViewerBehavior objects.
 *
 * InteractionSet is the concrete implementation of ImageViewerInteraction
 * used by ImageViewerWidget. It manages a collection of behaviors and
 * forwards input events to them.
 *
 * Event dispatch:
 * - Events are propagated to all registered behaviors
 * - A behavior may choose to handle (consume) an event
 * - A single behavior can enter a capturing state (exclusive interaction)
 *
 * Cursor management:
 * - The cursor is determined dynamically based on behaviors
 * - Active (capturing) behavior has priority
 * - Otherwise, available behaviors are queried
 *
 * Ownership:
 * - Behaviors are owned by this class (std::unique_ptr)
 * - InteractionSet is non-copyable but movable
 *
 * @note Behaviors are evaluated in insertion order unless priority
 *       is explicitly handled during dispatch.
 */
class InteractionSet : public ImageViewerInteraction
{
public:
    InteractionSet() = default;
    ~InteractionSet() override = default;

    // Non copyable (ownership of behaviors)
    InteractionSet(const InteractionSet&) = delete;
    InteractionSet& operator=(const InteractionSet&) = delete;

    // Movable if needed
    InteractionSet(InteractionSet&&) = default;
    InteractionSet& operator=(InteractionSet&&) = default;

    /**
     * @brief Adds a behavior to the interaction set.
     *      * The behavior is owned by the InteractionSet.
     *      * @param behavior Behavior to add.
     */
    void addBehavior(std::unique_ptr<ImageViewerBehavior> behavior);

    /**
     * @brief Returns the currently capturing behavior.
     *      * @return Pointer to the capturing behavior, or nullptr if none.
     */
    const ImageViewerBehavior* capturingBehavior() const;

    /**
     * @brief Computes the cursor for the current interaction state.
     *      * @param view Associated image viewer.
     * @param hasImage True if an image is loaded.
     * @param isPanRelevant True if panning is meaningful.
     * @param e Optional mouse event.
     * @return Cursor shape to display.
     */
    Qt::CursorShape cursor(const ImageViewerWidget& view, bool hasImage, bool isPanRelevant,
                           const QMouseEvent* e) const;

    /**
     * @brief Checks whether a behavior of a given type is present.
     *      * @tparam T Behavior type.
     * @return True if a behavior of type T is registered.
     */
    template <typename T> bool hasBehavior() const
    {
        for (const auto& b : behaviors_)
        {
            if (dynamic_cast<const T*>(b.get()))
                return true;
        }
        return false;
    }

protected:
    /**
     * @brief Handles wheel events.
     */
    bool wheel(ImageViewerWidget& view, QWheelEvent* event) override;

    /**
     * @brief Handles mouse press events.
     */
    bool mousePress(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse move events.
     */
    bool mouseMove(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse release events.
     */
    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse double-click events.
     */
    bool mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Computes the cursor for a given event.
     *      * Called internally by the interaction system.
     */
    Qt::CursorShape cursorForEvent(const ImageViewerWidget& view, bool hasImage, bool isPanRelevant,
                                   const QMouseEvent* event) const override;

    /**
     * @brief Handles drag enter events.
     */
    bool dragEnter(ImageViewerWidget& view, QDragEnterEvent* event) override;

    /**
     * @brief Handles drag move events.
     */
    bool dragMove(ImageViewerWidget& view, QDragMoveEvent* event) override;

    /**
     * @brief Handles drag leave events.
     */
    bool dragLeave(ImageViewerWidget& view, QDragLeaveEvent* event) override;

    /**
     * @brief Handles drop events.
     */
    bool drop(ImageViewerWidget& view, QDropEvent* event) override;

    /**
     * @brief Cancels all active behaviors.
     *      * Typically used to reset interaction state.
     */
    void cancel() override;

private:
    /// Owned behaviors composing the interaction system.
    std::vector<std::unique_ptr<ImageViewerBehavior>> behaviors_;
};

} // namespace fluvel_app
