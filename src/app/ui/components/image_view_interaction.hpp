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

    virtual bool wheel(ImageView&, QWheelEvent*) { return false; }
    virtual bool mousePress(ImageView&, QMouseEvent*) { return false; }
    virtual bool mouseMove(ImageView&, QMouseEvent*) { return false; }
    virtual bool mouseRelease(ImageView&, QMouseEvent*) { return false; }
    virtual bool mouseDoubleClick(ImageView&, QMouseEvent*) { return false; }

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
