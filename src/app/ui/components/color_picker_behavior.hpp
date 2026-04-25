// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"
#include "image_viewer_widget.hpp"
#include <QColor>

class QMouseEvent;
class QColor;
class QPoint;

namespace fluvel_app
{

class ImageViewerWidget;

/**
 * @brief Behavior for picking pixel colors from the image.
 *
 * This behavior allows the user to select a pixel color by clicking
 * on the image with a configurable mouse button (right button by default).
 *
 * When triggered, it emits a colorPicked() signal with the selected
 * color and image coordinates.
 *
 * Cursor:
 * - Displays a cross cursor when an image is available
 *
 * Priority:
 * - Medium priority to coexist with other interaction behaviors
 */
class ColorPickerBehavior : public QObject, public ImageViewerBehavior
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the color picker behavior.
     *      * @param button Mouse button used to pick a color
     *               (default: Qt::RightButton).
     */
    explicit ColorPickerBehavior(Qt::MouseButton button = Qt::RightButton);

    Qt::CursorShape availableCursor(bool hasImage, bool /*isZoomed*/, const ImageViewerWidget&,
                                    const QMouseEvent*) const override
    {
        return hasImage ? Qt::CrossCursor : Qt::ArrowCursor;
    }

    int priority() const override
    {
        return 30;
    }

signals:
    /**
     * @brief Emitted when a color is picked from the image.
     *      * @param color Picked color.
     * @param imgPos Position in image coordinates.
     */
    void colorPicked(const QColor& color, const QPoint& imgPos);

protected:
    /**
     * @brief Handles mouse press events.
     *      * Picks the color at the clicked position and emits colorPicked().
     */
    bool mousePress(ImageViewerWidget& view, QMouseEvent* e) override;

private:
    Qt::MouseButton button_;
};

} // namespace fluvel_app
