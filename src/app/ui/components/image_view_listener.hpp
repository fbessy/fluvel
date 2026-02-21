#ifndef IMAGE_VIEW_LISTENER_HPP
#define IMAGE_VIEW_LISTENER_HPP

#include <QColor>
#include <QPoint>

namespace ofeli_app
{

class ImageViewListener
{
public:
    virtual ~ImageViewListener() = default;
    virtual void onColorPicked(const QColor& color,
                               const QPoint& imagePos) = 0;
};

}

#endif // IMAGE_VIEW_LISTENER_HPP
