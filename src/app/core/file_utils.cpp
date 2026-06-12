#include "file_utils.hpp"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QMediaFormat>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>

namespace fluvel::file_utils
{

QString buildImageFilter()
{
    QStringList patterns;

    const auto formats = QImageReader::supportedImageFormats();
    for (const QByteArray& format : formats)
    {
        patterns << "*." + QString::fromLatin1(format);
    }

    patterns.sort();

    return QObject::tr("Image Files (%1)").arg(patterns.join(' '));
}

QString supportedImageExtensions()
{
    QStringList patterns;

    const auto formats = QImageReader::supportedImageFormats();

    for (const QByteArray& format : formats)
    {
        patterns << "*." + QString::fromLatin1(format);
    }

    patterns.sort();

    return patterns.join(' ');
}

QString normalizeImageFormat(QString format)
{
    format = format.toLower();

    if (format == "jpeg")
        return "jpg";

    if (format == "tiff")
        return "tif";

    return format;
}

QString buildWritableImageFilter()
{
    QStringList filters;

    const auto formats = QImageWriter::supportedImageFormats();

    for (const QByteArray& format : formats)
    {
        const QString ext = QString::fromLatin1(format).toLower();

        filters << QString("%1 (*.%2)").arg(ext.toUpper(), ext);
    }

    filters.sort();

    return filters.join(";;");
}

QString defaultExtensionFromFilter(const QString& selectedFilter)
{
    static const QRegularExpression kExtensionRegex(R"(\*\.(\w+))");

    QRegularExpressionMatch match = kExtensionRegex.match(selectedFilter);

    if (match.hasMatch())
        return match.captured(1);

    return {};
}

bool isSupportedImage(const QString& path)
{
    QImageReader reader(path);
    return reader.canRead();
}

QString strippedName(const QString& fullFilename)
{
    return QFileInfo(fullFilename).fileName();
}

QString makeUniqueFileName(const QString& filePath)
{
    QFileInfo fi(filePath);
    QString base = fi.completeBaseName();
    QString ext = fi.suffix();
    QDir dir = fi.dir();

    QString candidate = filePath;
    int index = 1;

    while (QFile::exists(candidate))
    {
        candidate = dir.filePath(QString("%1 (%2).%3").arg(base).arg(index++).arg(ext));
    }

    return candidate;
}

static QStringList extensionsForMediaFormat(QMediaFormat::FileFormat format)
{
    switch (format)
    {
        case QMediaFormat::MPEG4:
            return {"*.mp4", "*.m4v"};

        case QMediaFormat::Matroska:
            return {"*.mkv"};

        case QMediaFormat::AVI:
            return {"*.avi"};

        case QMediaFormat::QuickTime:
            return {"*.mov"};

        case QMediaFormat::WebM:
            return {"*.webm"};

        default:
            return {};
    }
}

QString supportedVideoExtensions()
{
    QStringList patterns;

    QMediaFormat mediaFormat;

    const auto formats = mediaFormat.supportedFileFormats(QMediaFormat::Decode);

    for (auto format : formats)
    {
        patterns.append(extensionsForMediaFormat(format));
    }

    patterns.removeDuplicates();
    patterns.sort();

    return patterns.join(' ');
}

QString buildVideoFilter()
{
    return QObject::tr("Video Files (%1)").arg(supportedVideoExtensions());
}
}
