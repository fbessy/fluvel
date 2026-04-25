// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QPoint>

namespace fluvel_app
{

/**
 * @brief Interface for receiving events from ImageViewerWidget.
 *
 * This listener allows external components to react to user interactions
 * occurring inside the image viewer.
 *
 * Implementations typically handle events such as color picking or
 * pixel inspection.
 *
 * @note The listener is not owned by the viewer.
 *       The caller must ensure its lifetime.
 *
 */

class ImageViewerListener
{
public:
    /// Virtual destructor.
    virtual ~ImageViewerListener() = default;

    /**
     * @brief Called when a pixel color is picked.
     *      * @param color Picked color value.
     * @param imagePos Position in image coordinates.
     */
    virtual void onColorPicked(const QColor& color, const QPoint& imagePos) = 0;
};

} // namespace fluvel_app
