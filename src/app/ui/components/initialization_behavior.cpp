// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "initialization_behavior.hpp"

#include "image_view.hpp"

#include <QMouseEvent>

namespace fluvel_app
{

InitializationBehavior::InitializationBehavior(QObject* parent)
    : QObject(parent)
{
}

bool InitializationBehavior::mouseMove(ImageView& view, QMouseEvent* e)
{
    const QPoint imgPos = view.imageCoordinatesFromView(e->pos());

    if (imgPos.x() < 0)
        return false;

    emit previewShapeRequested(imgPos);

    e->accept();
    return true;
}

bool InitializationBehavior::mousePress(ImageView& view, QMouseEvent* e)
{
    const QPoint imgPos = view.imageCoordinatesFromView(e->pos());

    if (imgPos.x() < 0)
        return false;

    if (e->button() == Qt::MiddleButton)
    {
        emit toggleShapeRequested();
        e->accept();
        return true;
    }

    if (e->button() == Qt::LeftButton)
    {
        emit addShapeRequested(imgPos);
        e->accept();
        return true;
    }

    if (e->button() == Qt::RightButton)
    {
        emit removeShapeRequested(imgPos);
        e->accept();
        return true;
    }

    return false;
}

bool InitializationBehavior::wheel(ImageView& /* view */, QWheelEvent* we)
{
    if (!(we->modifiers() & Qt::ControlModifier))
        return false;

    int delta = we->angleDelta().y();

    emit resizeShapeRequested(delta);

    we->accept();
    return true;
}

} // namespace fluvel_app
