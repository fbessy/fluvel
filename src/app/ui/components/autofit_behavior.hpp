// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"

class QMouseEvent;

namespace fluvel_app
{

/**
 * @brief Behavior triggering automatic view fitting.
 *
 * This behavior resets the view to fit the image within the viewport
 * when the configured mouse button is released (middle button by default).
 */
class AutoFitBehavior : public ImageViewerBehavior
{
public:
    /**
     * @brief Constructs the auto-fit behavior.
     *      * @param button Mouse button used to trigger auto-fit
     *               (default: Qt::MiddleButton).
     */
    explicit AutoFitBehavior(Qt::MouseButton button = Qt::MiddleButton);

    /**
     * @brief Handles mouse release events.
     *      * Triggers automatic fitting of the image to the view.
     */
    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;

private:
    Qt::MouseButton button_;
};

} // namespace fluvel_app
