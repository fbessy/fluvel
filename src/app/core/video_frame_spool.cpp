// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_frame_spool.hpp"

#include <QDebug>
#include <QImage>

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

bool VideoFrameSpool::clear()
{
    if (!spoolFile_.isOpen())
    {
        qWarning() << "Cannot clear spool file: spool file is not open.";
        return false;
    }

    if (!spoolFile_.resize(0))
    {
        qWarning() << "Failed to truncate spool file:" << spoolFile_.errorString();
        return false;
    }

    if (!spoolFile_.seek(0))
    {
        qWarning() << "Failed to seek to beginning of spool file:" << spoolFile_.errorString();
        return false;
    }

    return true;
}

bool VideoFrameSpool::isOpen() const
{
    return spoolFile_.isOpen();
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

    const qint64 position = spoolFile_.pos();

    if (position < 0)
    {
        qWarning() << "Failed to retrieve spool file position:" << spoolFile_.errorString();
        return std::nullopt;
    }

    const quint64 offset = static_cast<quint64>(position);

    if (spoolFile_.write(reinterpret_cast<const char*>(&header), sizeof(header)) != sizeof(header))
    {
        qWarning() << "Failed to write frame header:" << spoolFile_.errorString();
        return std::nullopt;
    }

    if (spoolFile_.write(reinterpret_cast<const char*>(frame.image.constBits()),
                         header.imageSize) != static_cast<qint64>(header.imageSize))
    {
        qWarning() << "Failed to write frame image:" << spoolFile_.errorString();
        return std::nullopt;
    }

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

    return frame;
}

} // namespace fluvel