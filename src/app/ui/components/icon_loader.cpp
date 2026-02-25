#include "icon_loader.hpp"
#include <QApplication>

namespace ofeli_app
{

// icon loader namespace for function outside a class
namespace il
{

QIcon loadIcon(const QString& themeName, QStyle::StandardPixmap standardName,
               const QString& fallback)
{
    QIcon icon;

    icon = QIcon::fromTheme(themeName + "-symbolic");
    if (!icon.isNull())
        return icon;

    icon = QIcon::fromTheme(themeName);
    if (!icon.isNull())
        return icon;

    icon = qApp->style()->standardIcon(standardName);
    if (!icon.isNull())
        return icon;

    return QIcon(fallback);
}

QIcon loadIcon(QIcon::ThemeIcon iconEnum, QStyle::StandardPixmap standardName,
               const QString& fallback)
{
    QIcon icon;

    icon = QIcon::fromTheme(iconEnum);
    if (!icon.isNull())
        return icon;

    icon = qApp->style()->standardIcon(standardName);
    if (!icon.isNull())
        return icon;

    return QIcon(fallback);
}

QIcon loadIcon(const QString& themeName, const QString& fallback)
{
    QIcon icon;

    icon = QIcon::fromTheme(themeName + "-symbolic");
    if (!icon.isNull())
        return icon;

    icon = QIcon::fromTheme(themeName);
    if (!icon.isNull())
        return icon;

    return QIcon(fallback);
}

QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback)
{
    QIcon icon;

    icon = QIcon::fromTheme(iconEnum);
    if (!icon.isNull())
        return icon;

    return QIcon(fallback);
}

} // namespace il

} // namespace ofeli_app
