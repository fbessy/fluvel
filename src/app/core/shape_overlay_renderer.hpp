#ifndef SHAPE_OVERLAY_RENDERER_HPP
#define SHAPE_OVERLAY_RENDERER_HPP

#include <QWidget>

#include "shape_type.hpp"
#include "phi_editor.hpp"

namespace ofeli_app
{

class ShapeOverlayRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit ShapeOverlayRenderer(QWidget* parent = nullptr);

    void setShape(const ShapeInfo& shape);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ShapeInfo shape_;
    QRect lastBoundingBox_;
};

}

#endif // SHAPE_OVERLAY_RENDERER_HPP
