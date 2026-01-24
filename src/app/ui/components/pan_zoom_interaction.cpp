#include "pan_zoom_interaction.hpp"

namespace ofeli_app
{

void PanZoomInteraction::wheel(ImageView& view,
                               ::QWheelEvent* event)
{
    constexpr double zoomFactor = 1.15;

    const QPointF scenePosBefore =
        view.mapToScene(event->position().toPoint());

    const double factor = (event->angleDelta().y() > 0)
                              ? zoomFactor
                              : 1.0 / zoomFactor;

    const double currentZoom = view.currentZoom();
    const double newZoom     = currentZoom * factor;

    if (newZoom < minZoom || newZoom > maxZoom)
        return;

    view.scaleView(factor, factor);

    const QPointF scenePosAfter =
        view.mapToScene(event->position().toPoint());

    const QPointF delta = scenePosAfter - scenePosBefore;
    view.translateView(delta.x(), delta.y());

    view.enableAutoView(false);
    view.updateDragMode();
}


void PanZoomInteraction::mousePress(ImageView& view,
                                    ::QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        view.enableAutoView(true);
        view.applyAutoView();
        view.updateDragMode();
    }
}

void PanZoomInteraction::mouseDoubleClick(ImageView& view,
                                          ::QMouseEvent*)
{
    view.toggleFullScreen();

    view.enableAutoView(true);
    view.applyAutoView();
    view.updateDragMode();
}

}
