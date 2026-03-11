// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_debug.hpp"

#include <QCryptographicHash>
#include <QDebug>

namespace fluvel_app::image_debug
{

QByteArray imageHash(const QImage& img)
{
    QImage normalized = img.convertToFormat(QImage::Format_RGBA8888);

    QCryptographicHash hash(QCryptographicHash::Sha256);

    hash.addData(QByteArrayView(reinterpret_cast<const char*>(normalized.constBits()),
                                normalized.sizeInBytes()));

    return hash.result();
}

std::string hexHash(const QImage& img)
{
    return imageHash(img).toHex().toStdString();
}

std::ostream& operator<<(std::ostream& os, const QImage& img)
{
    os << "QImage(" << img.width() << "x" << img.height()
       << " hash=" << fluvel_app::image_debug::hexHash(img) << ")";

    return os;
}

QDebug operator<<(QDebug dbg, const QImage& img)
{
    QDebugStateSaver saver(dbg);

    dbg.nospace() << "QImage(" << img.width() << "x" << img.height()
                  << " hash=" << QString::fromStdString(fluvel_app::image_debug::hexHash(img))
                  << ")";

    return dbg;
}

QString describeImage(const QImage& img)
{
    QByteArray hex = imageHash(img).toHex();

    return QString("%1x%2 %3").arg(img.width()).arg(img.height()).arg(QString(hex));
}

} // namespace fluvel_app::image_debug
