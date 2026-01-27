#ifndef COLOR_PICKER_BEHAVIOR_HPP
#define COLOR_PICKER_BEHAVIOR_HPP

#include "view_behavior.hpp"
#include "image_view.hpp"

#include <functional>

#include <QColor>

class QMouseEvent;
class QColor;
class QPoint;

namespace ofeli_app
{

class ImageView;

class ColorPickerBehavior : public ViewBehavior
{
public:
    explicit ColorPickerBehavior(Qt::MouseButton button = Qt::RightButton);

    Qt::CursorShape availableCursor(
        bool hasImage,
        bool /*isZoomed*/,
        const ImageView&,
        const QMouseEvent*) const override
    {
        return hasImage ? Qt::CrossCursor : Qt::ArrowCursor;
    }

    int priority() const override { return 30; }

protected:
    void mousePress(ImageView& view, QMouseEvent* e) override;

private:
    Qt::MouseButton button_;

signals:
    void colorPicked(const QColor& color, const QPoint& imgPos);
};

}

#endif // COLOR_PICKER_BEHAVIOR_HPP
