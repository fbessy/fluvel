// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QByteArray>
#include <QImage>

#include <ostream>

namespace fluvel_app::image_debug
{

QByteArray imageHash(const QImage& img);

std::string hexHash(const QImage& img);

std::ostream& operator<<(std::ostream& os, const QImage& img);

QString describeImage(const QImage& img);

} // namespace fluvel_app::image_debug
