// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "pixel_info_behavior.hpp"

#include "pixel_info_overlay.hpp"
#include "image_view.hpp"

#include <QtGui/qrgb.h>
#include <QGraphicsScene>
#include <QMouseEvent>

namespace ofeli_app
{

PixelInfoBehavior::PixelInfoBehavior() = default;

PixelInfoBehavior::~PixelInfoBehavior() = default;

// ---------------------------------------------------------------------

bool PixelInfoBehavior::mousePress(ImageView& view, QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return false;

    active_ = true;

    if (!overlay_)
        overlay_ = std::make_unique<PixelInfoOverlay>(view.graphicsScene());

    updateOverlay(view, event->pos());
    overlay_->showOverlay();

    return true;
}

// ---------------------------------------------------------------------

bool PixelInfoBehavior::mouseMove(ImageView& view, QMouseEvent* event)
{
    if (!active_)
        return false;

    if (!(event->buttons() & Qt::RightButton))
        return false;

    updateOverlay(view, event->pos());

    return true;
}

// ---------------------------------------------------------------------

bool PixelInfoBehavior::mouseRelease(ImageView&, QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return false;

    active_ = false;

    if (overlay_)
        overlay_->hideOverlay();

    return true;
}

// ---------------------------------------------------------------------

void PixelInfoBehavior::updateOverlay(ImageView& view, const QPoint& viewPos)
{
    if (!overlay_)
        return;

    const QPoint pixel = view.imageCoordinatesFromView(viewPos);
    if (pixel.x() < 0)
        return;

    const QRgb color = view.pixelColorAt(pixel);

    // position ancre : exactement sous le curseur
    const QPointF anchorScenePos = view.mapToScene(viewPos);

    overlay_->updateInfo(pixel, color, view.isGrayscale(), anchorScenePos, view);
}

} // namespace ofeli_app
