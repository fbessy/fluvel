// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_frame_spool.hpp"
#include "video_frame_header.hpp"

#include <QDebug>
#include <QImage>
#include <QStorageInfo>

namespace fluvel
{

VideoFrameSpool::VideoFrameSpool()
{
}

VideoFrameSpool::~VideoFrameSpool()
{
    close();
}

bool VideoFrameSpool::open()
{
    close();

    writeOffset_ = 0;

    if (!temporaryDirectory_.isValid())
    {
        qWarning() << "Failed to create temporary spool directory.";
        return false;
    }

    spoolFile_.setFileName(temporaryDirectory_.filePath("spool.bin"));

    if (!spoolFile_.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qWarning() << "Failed to open spool file:" << spoolFile_.fileName() << ":"
                   << spoolFile_.errorString();
        return false;
    }

    QStorageInfo storage(spoolFile_.fileName());

    qDebug() << "Spool filesystem:" << storage.rootPath()
             << "available:" << storage.bytesAvailable() << "bytes";

    return true;
}

void VideoFrameSpool::close()
{
    if (!spoolFile_.isOpen())
        return;

    if (!spoolFile_.flush())
        qWarning() << "Failed to flush spool file:" << spoolFile_.errorString();

    spoolFile_.close();

    if (spoolFile_.error() != QFileDevice::NoError)
        qWarning() << "Failed to close spool file:" << spoolFile_.errorString();

    spoolFile_.unsetError();
}

bool VideoFrameSpool::isOpen() const
{
    return spoolFile_.isOpen();
}

quint64 VideoFrameSpool::size() const
{
    if (!spoolFile_.isOpen())
        return 0;

    const qint64 size = spoolFile_.size();

    return size >= 0 ? static_cast<quint64>(size) : 0;
}

bool VideoFrameSpool::reset()
{
    if (!spoolFile_.isOpen())
    {
        qWarning() << "Cannot clear spool file: spool file is not open.";
        return false;
    }

    const QString fileName = spoolFile_.fileName();

    spoolFile_.close();

    if (!QFile::remove(fileName))
    {
        qWarning() << "Failed to remove spool file:" << fileName;
        return false;
    }

    spoolFile_.setFileName(fileName);

    if (!spoolFile_.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        qWarning() << "Failed to reopen spool file:" << spoolFile_.errorString();
        return false;
    }

    writeOffset_ = 0;

    return true;
}

bool VideoFrameSpool::remove()
{
    if (spoolFile_.isOpen())
        spoolFile_.close();

    if (spoolFile_.fileName().isEmpty())
        return true;

    if (QFile::exists(spoolFile_.fileName()) && !QFile::remove(spoolFile_.fileName()))
    {
        qWarning() << "Failed to remove spool file:" << spoolFile_.fileName();
        return false;
    }

    writeOffset_ = 0;

    return true;
}

std::optional<FrameLocation> VideoFrameSpool::write(const VideoFrame& frame)
{
    if (!spoolFile_.isOpen())
    {
        qWarning() << "Cannot write frame: spool file is not open.";
        return std::nullopt;
    }

    if (frame.image.isNull())
    {
        qWarning() << "Cannot write null image to spool.";

        return std::nullopt;
    }

    switch (frame.image.format())
    {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_Grayscale8:
            break;

        default:
            qWarning() << "Unsupported image format:" << frame.image.format();

            return std::nullopt;
    }

    VideoFrameHeader header;

    header.width = frame.image.width();
    header.height = frame.image.height();
    header.imageFormat = static_cast<quint32>(frame.image.format());
    header.imageSize = static_cast<quint64>(frame.image.sizeInBytes());
    header.hasPresentationTimestamp = frame.presentationTimestampNs.has_value();

    if (header.hasPresentationTimestamp)
        header.presentationTimestampNs = *frame.presentationTimestampNs;

    Q_ASSERT(header.imageSize == frame.image.sizeInBytes());
    Q_ASSERT(header.width > 0);
    Q_ASSERT(header.height > 0);

    const quint64 offset = writeOffset_;

    if (!spoolFile_.seek(static_cast<qint64>(writeOffset_)))
    {
        qWarning() << "Failed to seek to write position:" << spoolFile_.errorString();
        return std::nullopt;
    }

    Q_ASSERT(spoolFile_.pos() == static_cast<qint64>(offset));

    constexpr qint64 headerSize = static_cast<qint64>(sizeof(VideoFrameHeader));
    const qint64 imageSize = static_cast<qint64>(header.imageSize);

    if (spoolFile_.write(reinterpret_cast<const char*>(&header), headerSize) != headerSize)
    {
        qWarning() << "Failed to write frame header:" << spoolFile_.errorString();
        return std::nullopt;
    }

    if (spoolFile_.write(reinterpret_cast<const char*>(frame.image.constBits()), imageSize) !=
        imageSize)
    {
        qWarning() << "Failed to write frame image:" << spoolFile_.errorString();
        return std::nullopt;
    }

    const auto expected =
        static_cast<qint64>(offset + sizeof(VideoFrameHeader) + frame.image.sizeInBytes());

    Q_ASSERT(spoolFile_.pos() == expected);
    Q_ASSERT(spoolFile_.size() >= expected);

    // qDebug() << "queued =" << header.imageSize << "pos =" << spoolFile_.pos()
    // << "size =" << spoolFile_.size();

    if (spoolFile_.error() != QFileDevice::NoError)
    {
        qWarning() << "Spool file error:" << spoolFile_.errorString();
        return std::nullopt;
    }

    writeOffset_ += static_cast<quint64>(headerSize) + header.imageSize;

    return FrameLocation{offset};
}

std::optional<VideoFrame> VideoFrameSpool::read(const FrameLocation& location)
{
    if (!spoolFile_.isOpen())
    {
        qWarning() << "Cannot read frame: spool file is not open.";
        return std::nullopt;
    }

    if (!spoolFile_.seek(location.offset))
    {
        qWarning() << "Failed to seek to frame at offset" << location.offset << ":"
                   << spoolFile_.errorString();

        return std::nullopt;
    }

    VideoFrameHeader header;

    if (spoolFile_.read(reinterpret_cast<char*>(&header), sizeof(header)) != sizeof(header))
    {
        qWarning() << "Failed to read video frame header:" << spoolFile_.errorString();

        return std::nullopt;
    }

    Q_ASSERT(header.width > 0);
    Q_ASSERT(header.height > 0);
    Q_ASSERT(header.imageSize > 0);

    VideoFrame frame;

    const auto format = static_cast<QImage::Format>(header.imageFormat);

    switch (format)
    {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_Grayscale8:
            break;

        default:
            qWarning() << "Unsupported image format in spool file:" << header.imageFormat;

            return std::nullopt;
    }

    frame.image = QImage(header.width, header.height, format);

    if (frame.image.isNull())
    {
        qWarning() << "Failed to allocate image" << header.width << "x" << header.height;

        return std::nullopt;
    }

    if (static_cast<quint64>(frame.image.sizeInBytes()) != header.imageSize)
    {
        qWarning() << "Unexpected image size in spool file."
                   << "Expected:" << frame.image.sizeInBytes() << "Read:" << header.imageSize;

        return std::nullopt;
    }

    if (spoolFile_.read(reinterpret_cast<char*>(frame.image.bits()), header.imageSize) !=
        static_cast<qint64>(header.imageSize))
    {
        qWarning() << "Failed to read image data:" << spoolFile_.errorString();

        return std::nullopt;
    }

    if (header.hasPresentationTimestamp)
        frame.presentationTimestampNs = header.presentationTimestampNs;
    else
        frame.presentationTimestampNs.reset();

    // qDebug() << "READ " << location.offset << header.presentationTimestampNs;

    return frame;
}

} // namespace fluvel