#ifndef SHAPE_OVERLAY_RENDERER_HPP
#define SHAPE_OVERLAY_RENDERER_HPP

#include <QWidget>

#include "phi_editor.hpp"
#include "shape_type.hpp"

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

} // namespace ofeli_app

#endif // SHAPE_OVERLAY_RENDERER_HPP
