// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_debug.hpp"
#include "qimage_utils.hpp"

#include <QDebug>

namespace fluvel_app::image_debug
{

std::ostream& operator<<(std::ostream& os, const QImage& img)
{
    os << "QImage(" << img.width() << "x" << img.height() << " hash=" << qimage_utils::hexHash(img)
       << ")";

    return os;
}

QDebug operator<<(QDebug dbg, const QImage& img)
{
    QDebugStateSaver saver(dbg);

    dbg.nospace() << "QImage(" << img.width() << "x" << img.height()
                  << " hash=" << QString::fromStdString(qimage_utils::hexHash(img)) << ")";

    return dbg;
}

QString describeImage(const QImage& img)
{
    QByteArray hex = qimage_utils::imageHash(img).toHex();

    return QString("%1x%2 %3").arg(img.width()).arg(img.height()).arg(QString(hex));
}

} // namespace fluvel_app::image_debug
