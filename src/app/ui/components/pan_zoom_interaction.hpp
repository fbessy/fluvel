#ifndef PAN_ZOOM_INTERACTION_HPP
#define PAN_ZOOM_INTERACTION_HPP

#include "image_view_interaction.hpp"
#include "image_view.hpp"

#include <QWheelEvent>
#include <QMouseEvent>

namespace ofeli_app
{

class PanZoomInteraction : public ImageViewInteraction
{
protected:
    void wheel(ImageView& view, QWheelEvent* event) override;
    void mousePress(ImageView& view, QMouseEvent* event) override;
    void mouseDoubleClick(ImageView& view, QMouseEvent* event) override;

private:
    const double minZoom = 0.1;
    const double maxZoom = 20.0;
};

}

#endif // PAN_ZOOM_INTERACTION_HPP
