#include "file_utils.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QMediaFormat>
#include <QObject>
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

    for (const QByteArray& format : writableImageFormats())
    {
        const QString ext = QString::fromLatin1(format).toLower();

        filters << QString("%1 (*.%2)").arg(ext.toUpper(), ext);
    }

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
    for (const auto& info : detail::kVideoFormats)
    {
        if (info.format == format)
            return info.extensions;
    }

    return {};
}

QString supportedVideoExtensions()
{
    QStringList patterns;

    QMediaFormat mediaFormat;

    const auto formats = mediaFormat.supportedFileFormats(QMediaFormat::Decode);

    for (auto format : formats)
    {
        const auto extensions = extensionsForMediaFormat(format);

        for (const auto& ext : extensions)
        {
            patterns << "*." + ext;
        }
    }

    patterns.removeDuplicates();
    patterns.sort();

    return patterns.join(' ');
}

QString buildVideoFilter()
{
    return QObject::tr("Video Files (%1)").arg(supportedVideoExtensions());
}

bool isSupportedVideoFile(const QString& path)
{
    QFileInfo fi(path);

    if (!fi.isFile())
        return false;

    const QString suffix = "*." + fi.suffix().toLower();

    QStringList extensions = supportedVideoExtensions().split(' ');

    return extensions.contains(suffix, Qt::CaseInsensitive);
}

QString supportedMediaContainers()
{
    QStringList formats;

    QMediaFormat mediaFormat;

    const auto supported = mediaFormat.supportedFileFormats(QMediaFormat::Decode);

    for (auto format : supported)
    {
        const auto extensions = extensionsForMediaFormat(format);

        for (const auto& ext : extensions)
        {
            formats << ext.toUpper();
        }
    }

    formats.removeDuplicates();
    formats.sort();

    return formats.join(", ");
}

static QString prettyVideoCodecName(QMediaFormat::VideoCodec codec)
{
    switch (codec)
    {
        case QMediaFormat::VideoCodec::MPEG1:
            return "MPEG-1";

        case QMediaFormat::VideoCodec::MPEG2:
            return "MPEG-2";

        case QMediaFormat::VideoCodec::MPEG4:
            return "MPEG-4";

        case QMediaFormat::VideoCodec::H264:
            return "H.264 / AVC";

        case QMediaFormat::VideoCodec::H265:
            return "H.265 / HEVC";

        case QMediaFormat::VideoCodec::VP8:
            return "VP8";

        case QMediaFormat::VideoCodec::VP9:
            return "VP9";

        case QMediaFormat::VideoCodec::AV1:
            return "AV1";

        case QMediaFormat::VideoCodec::Theora:
            return "Theora";

        case QMediaFormat::VideoCodec::WMV:
            return "WMV";

        case QMediaFormat::VideoCodec::MotionJPEG:
            return "Motion JPEG";

        case QMediaFormat::VideoCodec::Unspecified:
            return QObject::tr("Unspecified");

        default:
            return QMediaFormat::videoCodecName(codec);
    }
}

QString supportedVideoCodecs()
{
    QStringList codecs;

    QMediaFormat format;

    const auto supported = format.supportedVideoCodecs(QMediaFormat::Decode);

    for (auto codec : supported)
        codecs << prettyVideoCodecName(codec);

    codecs.removeDuplicates();
    codecs.sort();

    return codecs.join(", ");
}

static QString prettyAudioCodecName(QMediaFormat::AudioCodec codec)
{
    switch (codec)
    {
        case QMediaFormat::AudioCodec::MP3:
            return "MP3";

        case QMediaFormat::AudioCodec::AAC:
            return "AAC";

        case QMediaFormat::AudioCodec::AC3:
            return "AC-3";

        case QMediaFormat::AudioCodec::EAC3:
            return "E-AC-3";

        case QMediaFormat::AudioCodec::FLAC:
            return "FLAC";

        case QMediaFormat::AudioCodec::DolbyTrueHD:
            return "Dolby TrueHD";

        case QMediaFormat::AudioCodec::Opus:
            return "Opus";

        case QMediaFormat::AudioCodec::Vorbis:
            return "Vorbis";

        case QMediaFormat::AudioCodec::Wave:
            return "Wave";

        case QMediaFormat::AudioCodec::WMA:
            return "WMA";

        case QMediaFormat::AudioCodec::ALAC:
            return "Apple Lossless (ALAC)";

        case QMediaFormat::AudioCodec::Unspecified:
            return QObject::tr("Unspecified");

        default:
            return QMediaFormat::audioCodecName(codec);
    }
}

QString supportedAudioCodecs()
{
    QStringList codecs;

    QMediaFormat format;

    const auto supported = format.supportedAudioCodecs(QMediaFormat::Decode);

    for (auto codec : supported)
        codecs << prettyAudioCodecName(codec);

    codecs.removeDuplicates();
    codecs.sort();

    return codecs.join(", ");
}

QList<QByteArray> writableImageFormats()
{
    auto formats = QImageWriter::supportedImageFormats();

    std::sort(formats.begin(), formats.end());

    return formats;
}

QString buildOutputFileName(const QString& directory, const QString& baseName,
                            const QString& extension, bool appendTimestamp)
{
    QString name = baseName;

    if (appendTimestamp)
        name += QDateTime::currentDateTime().toString("_yyyy-MM-dd_HH-mm-ss");

    return makeUniqueFileName(QDir(directory).filePath(name + "." + extension));
}
}
