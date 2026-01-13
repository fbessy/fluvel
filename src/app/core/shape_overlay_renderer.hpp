#ifndef SHAPE_OVERLAY_RENDERER_HPP
#define SHAPE_OVERLAY_RENDERER_HPP

#include <QImage>
#include <QColor>

#include "shape_type.hpp"

namespace ofeli_app
{

struct ShapeParams
{
    ShapeType type;

    float center_x;   // [-0.5 .. +0.5] relatif image
    float center_y;

    float width;      // [0..1] relatif image
    float height;

    QColor color;
};

class ShapeOverlayRenderer
{
public:
    static QImage render(const QImage& base,
                         const ShapeParams& params);
};

}

#endif // SHAPE_OVERLAY_RENDERER_HPP
