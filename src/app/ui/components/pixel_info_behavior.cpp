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

    constexpr int dx = 12;
    constexpr int dy = 12;

    const QPoint overlayViewPos = viewPos + QPoint(dx, dy);
    const QPointF overlayScenePos = view.mapToScene(overlayViewPos);

    overlay_->updateInfo(pixel,
                         color,
                         view.isGrayscale(),
                         overlayScenePos);
}

} // namespace ofeli_app
