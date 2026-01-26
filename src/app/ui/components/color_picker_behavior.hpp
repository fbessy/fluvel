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
    explicit ColorPickerBehavior(bool singleShot = false);

    void mousePress(ImageView&, QMouseEvent*) override;

    std::function<void(const QColor&, const QPoint&)> onColorPicked;

private:
    bool singleShot_;
};

}

#endif // COLOR_PICKER_BEHAVIOR_HPP
