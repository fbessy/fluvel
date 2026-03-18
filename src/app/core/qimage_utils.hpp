// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QByteArray>
#include <QImage>

#include <string>

namespace fluvel_app::qimage_utils
{

QImage darkenImage(const QImage& image);

QByteArray imageHash(const QImage& image);
std::string hexHash(const QImage& image);

} // namespace qimage_utils
