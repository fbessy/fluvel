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

    float center_x;
    float center_y;

    float width;
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
