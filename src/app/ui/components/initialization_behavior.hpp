// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"
#include "view_behavior.hpp"

#include <QObject>
#include <QPoint>

class QMouseEvent;
class ImageView;

namespace fluvel_app
{

class InitializationBehavior : public QObject, public ViewBehavior
{
    Q_OBJECT

public:
    explicit InitializationBehavior(QObject* parent = nullptr);

    bool mouseMove(ImageView& view, QMouseEvent* e) override;
    bool mousePress(ImageView& view, QMouseEvent* e) override;
    bool wheel(ImageView& view, QWheelEvent* we) override;

signals:
    void previewShapeRequested(QPoint imagePos);
    void addShapeRequested(QPoint imagePos);
    void removeShapeRequested(QPoint imagePos);
    void resizeShapeRequested(int delta);
    void toggleShapeRequested();
};

} // namespace fluvel_app
