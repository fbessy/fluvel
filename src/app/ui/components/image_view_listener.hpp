// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QPoint>

namespace fluvel_app
{

class ImageViewListener
{
public:
    virtual ~ImageViewListener() = default;
    virtual void onColorPicked(const QColor& color, const QPoint& imagePos) = 0;
};

} // namespace fluvel_app
