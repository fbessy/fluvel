#ifndef ICON_LOADER_HPP
#define ICON_LOADER_HPP

#include <QIcon>
#include <QStyle>

namespace ofeli_app
{

namespace il
{

QIcon loadIcon(const QString& themeName,
               QStyle::StandardPixmap standardName,
               const QString& fallback);
QIcon loadIcon(QIcon::ThemeIcon iconEnum,
               QStyle::StandardPixmap standardName,
               const QString& fallback);
QIcon loadIcon(const QString& themeName,
               const QString& fallback);
QIcon loadIcon(QIcon::ThemeIcon iconEnum,
               const QString& fallback);
}

}

#endif // ICON_LOADER_HPP
