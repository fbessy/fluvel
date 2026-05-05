// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "phi_editor.hpp"

#include <QPainter>
#include <cmath>
#include <cstddef>

namespace fluvel_app
{

PhiEditor::PhiEditor(const QImage& phi)
    : committedPhi_(phi)
{
    editedPhi_ = committedPhi_;
}

void PhiEditor::loadPhi(const QImage& phi)
{
    committedPhi_ = phi;
    editedPhi_ = phi;

    emit editedPhiChanged();
}

QImage PhiEditor::commit()
{
    committedPhi_ = editedPhi_;

    return committedPhi_;
}

void PhiEditor::revert()
{
    editedPhi_ = committedPhi_;

    emit editedPhiChanged();
}

static void drawEllipseMask(QImage& img, const QRect& rect, uchar value)
{
    const QRect r = rect.normalized();

    const int cx = r.center().x();
    const int cy = r.center().y();
    const int rx = r.width() / 2;
    const int ry = r.height() / 2;

    if (rx <= 0 || ry <= 0)
        return;

    const int ry2 = ry * ry;
    const float invRy2 = 1.0f / static_cast<float>(ry2);

    for (int y = cy - ry; y <= cy + ry; ++y)
    {
        if (y < 0 || y >= img.height())
            continue;

        const int dy = y - cy;
        const int dy2 = dy * dy;

        // solve ellipse equation for x
        const float t = 1.0f - static_cast<float>(dy2) * invRy2;

        if (t < 0.0f)
            continue;

        const int dx = static_cast<int>(std::lround(static_cast<float>(rx) * std::sqrt(t)));

        const int x0 = std::max(0, cx - dx);
        const int x1 = std::min(img.width() - 1, cx + dx);

        uchar* line = img.scanLine(y);

        if (x0 <= x1)
            std::memset(line + x0, value, static_cast<size_t>(x1 - x0 + 1));
    }
}

static void drawRectMask(QImage& img, const QRect& rect, uchar value)
{
    QRect r = rect.normalized();

    for (int y = r.top(); y <= r.bottom(); ++y)
    {
        if (y < 0 || y >= img.height())
            continue;

        uchar* line = img.scanLine(y);

        for (int x = r.left(); x <= r.right(); ++x)
        {
            if (x < 0 || x >= img.width())
                continue;

            line[x] = value;
        }
    }
}

void PhiEditor::changeShape(const ShapeInfo& shapeInfo, const QColor& color)
{
    if (!shapeInfo.boundingBox.isValid())
        return;

    QRect rect = shapeInfo.boundingBox.normalized();

    uchar value = (color == Qt::white) ? 255 : 0;

    if (shapeInfo.type == ShapeType::Rectangle)
        drawRectMask(editedPhi_, rect, value);
    else if (shapeInfo.type == ShapeType::Ellipse)
        drawEllipseMask(editedPhi_, rect, value);
}

void PhiEditor::addShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::white);
    emit editedPhiChanged();
}

void PhiEditor::subtractShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::black);
    emit editedPhiChanged();
}

void PhiEditor::clear()
{
    editedPhi_.fill(Qt::black);
    emit editedPhiCleared();
}

void PhiEditor::setSize(const QSize& size)
{
    if (editedPhi_.isNull())
        return;

    if (size == editedPhi_.size())
        return;

    editedPhi_ = editedPhi_.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    committedPhi_ = committedPhi_.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    emit editedPhiChanged();
}

} // namespace fluvel_app
