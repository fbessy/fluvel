#include "shape_overlay_renderer.hpp"

#include "shape_overlay_renderer.hpp"
#include <QPainter>

namespace ofeli_app
{

QImage ShapeOverlayRenderer::render(const QImage& base,
                                    const ShapeParams& params)
{
    if (base.isNull())
        return base;

    QImage result = base.copy();

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(params.color);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    const int w = base.width();
    const int h = base.height();

    const float cx = (0.5f + params.center_x) * w;
    const float cy = (0.5f + params.center_y) * h;

    const float rw = params.width  * w;
    const float rh = params.height * h;

    QRectF rect(cx - rw * 0.5f,
                cy - rh * 0.5f,
                rw,
                rh);

    if (params.type == ShapeType::Ellipse)
        painter.drawEllipse(rect);
    else
        painter.drawRect(rect);

    return result;
}

} // namespace ofeli_app
