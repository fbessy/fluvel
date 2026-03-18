// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QImage>
#include <QString>

#include <iostream>

namespace fluvel_app::image_debug
{

std::ostream& operator<<(std::ostream& os, const QImage& img);

QString describeImage(const QImage& img);

} // namespace fluvel_app::image_debug
