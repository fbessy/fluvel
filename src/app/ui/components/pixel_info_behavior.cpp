// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "pixel_info_behavior.hpp"

#include "pixel_info_overlay.hpp"
#include "image_viewer_widget.hpp"

#include <QtGui/qrgb.h>
#include <QGraphicsScene>
#include <QMouseEvent>

namespace fluvel_app
{

// ---------------------------------------------------------------------

bool PixelInfoBehavior::mousePress(ImageViewerWidget& view, QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return false;

    if (!view.hasImage())
        return false;

    const QPoint pixel = view.imageCoordinatesFromView(event->pos());

    // 👇 clic hors image → on refuse d'activer
    if (pixel.x() < 0)
        return false;

    active_ = true;

    if (!overlay_)
        overlay_ = std::make_unique<PixelInfoOverlay>(view.scene());

    updateOverlay(view, event->pos());
    overlay_->showOverlay();

    event->accept();
    return true;
}

// ---------------------------------------------------------------------

bool PixelInfoBehavior::mouseMove(ImageViewerWidget& view, QMouseEvent* event)
{
    if (!active_)
        return false;

    // Si le bouton droit n'est plus enfoncé → on stoppe tout
    if (!(event->buttons() & Qt::RightButton))
    {
        active_ = false;

        if (overlay_)
            overlay_->hideOverlay();

        return false;
    }

    updateOverlay(view, event->pos());
    return true;
}

// ---------------------------------------------------------------------

bool PixelInfoBehavior::mouseRelease(ImageViewerWidget&, QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return false;

    active_ = false;

    if (overlay_)
        overlay_->hideOverlay();

    return true;
}

// ---------------------------------------------------------------------

void PixelInfoBehavior::updateOverlay(ImageViewerWidget& view, const QPoint& viewPos)
{
    if (!overlay_)
        return;

    const QPoint pixel = view.imageCoordinatesFromView(viewPos);

    if (!view.isPixelVisible(viewPos))
    {
        overlay_->hideOverlay();
        return;
    }

    QPointF scenePos = view.mapToScene(viewPos);

    QRectF visibleSceneRect = view.mapToScene(view.viewport()->rect()).boundingRect();

    if (!visibleSceneRect.contains(scenePos))
    {
        overlay_->hideOverlay();
        return;
    }

    const QRgb color = view.pixelColorAt(pixel);

    overlay_->updateInfo(pixel, color, view.isGrayscale(), scenePos, view);
}

void PixelInfoBehavior::cancel()
{
    active_ = false;

    if (overlay_)
        overlay_->hideOverlay();
}

} // namespace fluvel_app
