// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy
#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace fluvel_app::device
{

inline QString encodeDeviceId(const QByteArray& id)
{
    return QString::fromUtf8(QUrl::toPercentEncoding(id));
}

inline QByteArray decodeDeviceId(const QString& key)
{
    return QUrl::fromPercentEncoding(key.toUtf8()).toUtf8();
}

} // namespace fluvel_app::device
