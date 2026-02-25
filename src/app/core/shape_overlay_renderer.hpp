// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "phi_editor.hpp"

#include <QWidget>

namespace ofeli_app
{

class ShapeOverlayRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit ShapeOverlayRenderer(QWidget* parent = nullptr);

    void setShape(const ShapeInfo& shape);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ShapeInfo shape_;
    QRect lastBoundingBox_;
};

} // namespace ofeli_app
