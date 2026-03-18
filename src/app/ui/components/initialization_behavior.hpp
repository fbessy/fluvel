// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"
#include "image_viewer_behavior.hpp"

#include <QObject>
#include <QPoint>

class QMouseEvent;
class ImageViewerWidget;

namespace fluvel_app
{

class InitializationBehavior : public QObject, public ImageViewerBehavior
{
    Q_OBJECT

public:
    explicit InitializationBehavior(QObject* parent = nullptr);

    bool mouseMove(ImageViewerWidget& view, QMouseEvent* e) override;
    bool mousePress(ImageViewerWidget& view, QMouseEvent* e) override;
    bool wheel(ImageViewerWidget& view, QWheelEvent* we) override;

signals:
    void previewShapeRequested(QPoint imagePos);
    void addShapeRequested(QPoint imagePos);
    void removeShapeRequested(QPoint imagePos);
    void resizeShapeRequested(int delta);
    void toggleShapeRequested();
};

} // namespace fluvel_app
