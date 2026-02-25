// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "shape_type.hpp"

#include <QObject>
#include <QImage>
#include <QRect>

namespace ofeli_app
{

struct ShapeInfo
{
    ShapeType type;
    QRect boundingBox;
};

class PhiEditor : public QObject
{
    Q_OBJECT

public:
    PhiEditor(const QImage& committedPhi);

    void addShape(const ShapeInfo& shape);
    void subtractShape(const ShapeInfo& shape);
    void clear();

    void accept();
    void reject();

    void setSize(const QSize& size);

    const QImage& phi() const
    {
        return editedPhi_;
    }

signals:
    void phiAccepted(const QImage& committedPhi);
    void editedPhiChanged();
    void editedPhiCleared();

private:
    void changeShape(const ShapeInfo& shapeInfo, const QColor& color);

    QImage editedPhi_;
    QImage committedPhi_;
};

} // namespace ofeli_app
