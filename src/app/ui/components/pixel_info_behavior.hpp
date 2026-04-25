// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "pixel_info_overlay.hpp"
#include "image_viewer_behavior.hpp"
#include <QPoint>
#include <memory>

class QMouseEvent;

namespace fluvel_app
{

class ImageViewerWidget;

/**
 * @brief Behavior displaying pixel information while the right mouse button is held.
 *
 * This behavior shows a PixelInfoOverlay near the cursor, providing
 * pixel coordinates and color values from the underlying image.
 *
 * Activation:
 * - Triggered by right mouse button press
 * - Active only while the button is held
 * - Stops on mouse release or cancel()
 *
 * Characteristics:
 * - Does not consume events (returns false)
 * - Does not enforce exclusive capture
 * - Uses a cross cursor while active
 *
 * The overlay is created on demand and follows cursor movement.
 *
 * @note Optional behavior: if not added to the interaction set, it is disabled.
 */
class PixelInfoBehavior final : public ImageViewerBehavior
{
public:
    PixelInfoBehavior() = default;
    ~PixelInfoBehavior() override = default;

    bool isActive() const
    {
        return active_;
    }

    Qt::CursorShape activeCursor() const override
    {
        return Qt::CrossCursor;
    }

    bool isCapturing() const override
    {
        return active_;
    }

    /**
     * @brief Handles mouse press events.
     *      * Activates the behavior if the right mouse button is pressed
     * and initializes the overlay.
     *      * @param view Associated image viewer.
     * @param event Mouse event.
     * @return false (event is not consumed).
     */
    bool mousePress(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse move events.
     *      * Updates the overlay position and pixel information while active.
     *      * @param view Associated image viewer.
     * @param event Mouse event.
     * @return false (event is not consumed).
     */
    bool mouseMove(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Handles mouse release events.
     *      * Deactivates the behavior and hides the overlay when the right
     * mouse button is released.
     *      * @param view Associated image viewer.
     * @param event Mouse event.
     * @return false (event is not consumed).
     */
    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;

    /**
     * @brief Cancels the behavior.
     *      * Ensures the overlay is hidden and internal state is reset.
     */
    void cancel() override;

private:
    /**
     * @brief Updates overlay content and position from a view coordinate.
     *      * Converts the view position to image/scene space and refreshes
     * the overlay accordingly.
     *      * @param view Associated image viewer.
     * @param viewPos Mouse position in view coordinates.
     */
    void updateOverlay(ImageViewerWidget& view, const QPoint& viewPos);

    /// True while the behavior is active (right button held).
    bool active_{false};

    /// Overlay displaying pixel information (created on demand).
    std::unique_ptr<PixelInfoOverlay> overlay_;
};

} // namespace fluvel_app
