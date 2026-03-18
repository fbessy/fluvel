// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QString>

namespace fluvel_app::file_utils
{

QString buildImageFilter();
bool isSupportedImage(const QString& path);
QString strippedName(const QString& fullFilename);
QString makeUniqueFileName(const QString& filePath);

} // namespace fluvel_app::file_utils
