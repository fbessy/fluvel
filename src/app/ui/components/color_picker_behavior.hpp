// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"
#include "image_viewer_widget.hpp"
#include <QColor>

class QMouseEvent;
class QColor;
class QPoint;

namespace fluvel_app
{

class ImageViewerWidget;

class ColorPickerBehavior : public ImageViewerBehavior
{
public:
    explicit ColorPickerBehavior(Qt::MouseButton button = Qt::RightButton);

    Qt::CursorShape availableCursor(bool hasImage, bool /*isZoomed*/, const ImageViewerWidget&,
                                    const QMouseEvent*) const override
    {
        return hasImage ? Qt::CrossCursor : Qt::ArrowCursor;
    }

    int priority() const override
    {
        return 30;
    }

protected:
    bool mousePress(ImageViewerWidget& view, QMouseEvent* e) override;

private:
    Qt::MouseButton button_;

signals:
    void colorPicked(const QColor& color, const QPoint& imgPos);
};

} // namespace fluvel_app
