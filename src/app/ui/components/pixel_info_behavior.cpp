#include "pixel_info_behavior.hpp"

#include <QMouseEvent>
#include <QGraphicsScene>
#include <QColor>

#include "image_view.hpp"
#include "pixel_info_overlay.hpp"

namespace ofeli_app {

PixelInfoBehavior::PixelInfoBehavior() = default;

PixelInfoBehavior::~PixelInfoBehavior() = default;

// ---------------------------------------------------------------------

void PixelInfoBehavior::mousePress(ImageView& view,
                                   QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return;

    active_ = true;

    if (!overlay_)
        overlay_ = std::make_unique<PixelInfoOverlay>(view.graphicsScene());

    updateOverlay(view, event->pos());
    overlay_->showOverlay();
}

// ---------------------------------------------------------------------

void PixelInfoBehavior::mouseMove(ImageView& view,
                                  QMouseEvent* event)
{
    if (!active_)
        return;

    if (!(event->buttons() & Qt::RightButton))
        return;

    updateOverlay(view, event->pos());
}

// ---------------------------------------------------------------------

void PixelInfoBehavior::mouseRelease(ImageView&,
                                     QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return;

    active_ = false;

    if (overlay_)
        overlay_->hideOverlay();
}

// ---------------------------------------------------------------------

void PixelInfoBehavior::updateOverlay(ImageView& view,
                                      const QPoint& viewPos)
{
    if (!overlay_)
        return;

    const QPoint pixel = view.imageCoordinatesFromView(viewPos);
    if (pixel.x() < 0)
        return;

    const QRgb color = view.pixelColorAt(pixel);

    // position ancre : exactement sous le curseur
    const QPointF anchorScenePos = view.mapToScene(viewPos);

    overlay_->updateInfo(pixel,
                         color,
                         view.isGrayscale(),
                         anchorScenePos,
                         view);
}

} // namespace ofeli_app
