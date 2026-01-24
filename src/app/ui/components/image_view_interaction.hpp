#ifndef IMAGE_VIEW_INTERACTION_HPP
#define IMAGE_VIEW_INTERACTION_HPP

#include <QWheelEvent>
#include <QMouseEvent>

namespace ofeli_app
{

class ImageView;

class ImageViewInteraction
{
public:
    virtual ~ImageViewInteraction() = default;

    virtual void wheel(ImageView&, ::QWheelEvent*) {}
    virtual void mousePress(ImageView&, ::QMouseEvent*) {}
    virtual void mouseDoubleClick(ImageView&, ::QMouseEvent*) {}
};

} // namespace ofeli_app

#endif // IMAGE_VIEW_INTERACTION_HPP
