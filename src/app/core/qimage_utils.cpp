// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "qimage_utils.hpp"

#include <QCryptographicHash>
#include <QPainter>

namespace fluvel_app::qimage_utils
{

QImage darkenImage(const QImage& image)
{
    QImage dark = image.convertToFormat(QImage::Format_ARGB32);

    QPainter p(&dark);
    p.setCompositionMode(QPainter::CompositionMode_Multiply);

    // gris sombre plutôt que noir pur
    p.fillRect(dark.rect(), QColor(100, 100, 100));
    p.end();

    return dark;
}

QByteArray imageHash(const QImage& image)
{
    QImage normalized = image.convertToFormat(QImage::Format_RGBA8888);

    QCryptographicHash hash(QCryptographicHash::Sha256);

    hash.addData(QByteArrayView(reinterpret_cast<const char*>(normalized.constBits()),
                                normalized.sizeInBytes()));

    return hash.result();
}

std::string hexHash(const QImage& image)
{
    return imageHash(image).toHex().toStdString();
}

} // namespace qimage_utils
