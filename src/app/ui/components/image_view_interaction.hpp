#ifndef IMAGE_VIEW_INTERACTION_HPP
#define IMAGE_VIEW_INTERACTION_HPP

#include <QWheelEvent>
#include <QMouseEvent>

#include <QtCore/Qt>

namespace ofeli_app
{

class ImageView;

class ImageViewInteraction
{
public:
    virtual ~ImageViewInteraction() = default;

    virtual void wheel(ImageView&, QWheelEvent*) {}
    virtual void mousePress(ImageView&, QMouseEvent*) {}
    virtual void mouseMove(ImageView&, QMouseEvent*) {}
    virtual void mouseRelease(ImageView&, QMouseEvent*) {}
    virtual void mouseDoubleClick(ImageView&, QMouseEvent*) {}

    virtual Qt::CursorShape cursorForEvent(
        const ImageView& /*view*/,
        bool /*hasImage*/,
        bool /*isPanRelevant*/,
        const QMouseEvent* /*event*/) const
    {
        return Qt::ArrowCursor;
    }
};

} // namespace ofeli_app

#endif // IMAGE_VIEW_INTERACTION_HPP
