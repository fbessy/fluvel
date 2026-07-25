// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_frame_spool.hpp"
#include "video_frame_header.hpp"

#include <QDebug>
#include <QImage>
#include <QStorageInfo>

#include <cassert>

namespace
{

struct SegmentLayout
{
    quint64 segmentSize;
    int segmentCount;
};

[[nodiscard]]
SegmentLayout computeSegmentLayout(quint64 maximumSize)
{
    assert(maximumSize > 0);

    constexpr quint64 MiB = 1024ull * 1024ull;

    constexpr quint64 minSegmentSize = 64 * MiB;
    constexpr quint64 maxSegmentSize = 512 * MiB;
    constexpr int targetSegmentCount = 64;

    const auto segmentSize =
        std::clamp(maximumSize / targetSegmentCount, minSegmentSize, maxSegmentSize);

    const auto segmentCount = static_cast<int>((maximumSize + segmentSize - 1) / segmentSize);

    return {segmentSize, segmentCount};
}

} // namespace

namespace fluvel
{

VideoFrameSpool::~VideoFrameSpool()
{
    close();
}

void VideoFrameSpool::setMaximumSize(quint64 bytes)
{
    assert(bytes > 0);
    assert(!isOpen());

    maximumSize_ = bytes;

    const auto layout = computeSegmentLayout(bytes);

    segmentSize_ = layout.segmentSize;
    segmentCount_ = layout.segmentCount;
}

bool VideoFrameSpool::open()
{
    close();

    if (!temporaryDirectory_.isValid())
    {
        qWarning() << "Failed to create temporary spool directory.";
        return false;
    }

    assert(segmentCount_ > 0);
    assert(segmentSize_ > 0);

    segments_.clear();
    segments_.resize(segmentCount_);

    currentWriteSegment_ = 0;

    for (int i = 0; i < segmentCount_; ++i)
    {
        auto& segment = segments_[i];

        segment.writeOffset = 0;
        segment.refCount = 0;

        segment.file = std::make_unique<QFile>();

        segment.file->setFileName(
            temporaryDirectory_.filePath(QString("spool_%1.bin").arg(i, 4, 10, QChar('0'))));

        if (!segment.file->open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            qWarning() << "Failed to open spool segment:" << segment.file->fileName() << ":"
                       << segment.file->errorString();

            close();
            return false;
        }
    }

    QStorageInfo storage(temporaryDirectory_.path());

    qDebug() << "Spool filesystem:" << storage.rootPath()
             << "available:" << storage.bytesAvailable() << "bytes"
             << "segment count:" << segmentCount_ << "segment size:" << segmentSize_;

    return true;
}

void VideoFrameSpool::close()
{
    for (auto& segment : segments_)
    {
        if (!segment.file)
            continue;

        if (segment.file->isOpen())
        {
            if (!segment.file->flush())
            {
                qWarning() << "Failed to flush spool segment:" << segment.file->fileName()
                           << segment.file->errorString();
            }

            segment.file->close();

            if (segment.file->error() != QFileDevice::NoError)
            {
                qWarning() << "Failed to close spool segment:" << segment.file->fileName()
                           << segment.file->errorString();
            }

            segment.file->unsetError();
        }

        segment.writeOffset = 0;
        segment.refCount = 0;
    }

    segments_.clear();

    currentWriteSegment_ = 0;
}

bool VideoFrameSpool::isOpen() const
{
    return !segments_.empty() && segments_.front().file && segments_.front().file->isOpen();
}

quint64 VideoFrameSpool::size() const
{
    quint64 total = 0;

    for (const auto& segment : segments_)
        total += segment.writeOffset;

    return total;
}

bool VideoFrameSpool::reset()
{
    if (!isOpen())
    {
        qWarning() << "Cannot reset spool: spool is not open.";
        return false;
    }

    for (auto& segment : segments_)
    {
        assert(segment.file);

        if (!segment.file->resize(0))
        {
            qWarning() << "Failed to truncate spool segment:" << segment.file->fileName() << ":"
                       << segment.file->errorString();
            return false;
        }

        if (!segment.file->seek(0))
        {
            qWarning() << "Failed to seek spool segment:" << segment.file->fileName() << ":"
                       << segment.file->errorString();
            return false;
        }

        segment.writeOffset = 0;
        segment.refCount = 0;
    }

    currentWriteSegment_ = 0;

    return true;
}

std::optional<FrameLocation> VideoFrameSpool::write(const VideoFrame& frame)
{
    if (!isOpen())
    {
        qWarning() << "Cannot write frame: spool is not open.";
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

    constexpr quint64 headerSize = sizeof(VideoFrameHeader);
    const quint64 requiredSize = headerSize + header.imageSize;

    if (!prepareWriteSegment(requiredSize))
        return std::nullopt;

    auto& segment = segments_[currentWriteSegment_];

    const quint64 offset = segment.writeOffset;

    if (!segment.file->seek(static_cast<qint64>(offset)))
    {
        qWarning() << "Failed to seek:" << segment.file->errorString();
        return std::nullopt;
    }

    if (segment.file->write(reinterpret_cast<const char*>(&header), headerSize) != headerSize)
    {
        qWarning() << "Failed to write frame header:" << segment.file->errorString();
        return std::nullopt;
    }

    if (segment.file->write(reinterpret_cast<const char*>(frame.image.constBits()),
                            static_cast<qint64>(header.imageSize)) !=
        static_cast<qint64>(header.imageSize))
    {
        qWarning() << "Failed to write image:" << segment.file->errorString();
        return std::nullopt;
    }

    segment.writeOffset += requiredSize;
    ++segment.refCount;

    return FrameLocation{currentWriteSegment_, offset};
}

bool VideoFrameSpool::prepareWriteSegment(quint64 requiredSize)
{
    auto& currentSegment = segments_[currentWriteSegment_];

    // The current segment still has enough free space.
    if (currentSegment.writeOffset + requiredSize <= segmentSize_)
        return true;

    const int startSegment = currentWriteSegment_;
    bool found = false;

    for (int i = 1; i <= segmentCount_; ++i)
    {
        const int candidate = (startSegment + i) % segmentCount_;
        auto& segment = segments_[candidate];

        if (segment.refCount != 0)
            continue;

        if (!segment.file->resize(0))
        {
            qWarning() << "Failed to truncate spool segment:" << segment.file->fileName()
                       << segment.file->errorString();
            return false;
        }

        if (!segment.file->seek(0))
        {
            qWarning() << "Failed to seek spool segment:" << segment.file->fileName()
                       << segment.file->errorString();
            return false;
        }

        segment.writeOffset = 0;

        currentWriteSegment_ = candidate;
        found = true;
        break;
    }

    if (!found)
    {
        qWarning() << "No free spool segment available.";
        return false;
    }

    return true;
}

std::optional<VideoFrame> VideoFrameSpool::read(const FrameLocation& location)
{
    assert(location.segment >= 0);
    assert(static_cast<std::size_t>(location.segment) < segments_.size());

    if (!isOpen())
    {
        qWarning() << "Cannot read frame: spool is not open.";
        return std::nullopt;
    }

    auto& segment = segments_[location.segment];

    if (!segment.file->seek(static_cast<qint64>(location.offset)))
    {
        qWarning() << "Failed to seek:" << segment.file->errorString();
        return std::nullopt;
    }

    VideoFrameHeader header;

    if (segment.file->read(reinterpret_cast<char*>(&header), sizeof(header)) != sizeof(header))
    {
        qWarning() << "Failed to read frame header:" << segment.file->errorString();
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
            qWarning() << "Unsupported image format:" << header.imageFormat;
            return std::nullopt;
    }

    frame.image = QImage(header.width, header.height, format);

    if (frame.image.isNull())
    {
        qWarning() << "Failed to allocate image.";
        return std::nullopt;
    }

    if (static_cast<quint64>(frame.image.sizeInBytes()) != header.imageSize)
    {
        qWarning() << "Unexpected image size.";
        return std::nullopt;
    }

    if (segment.file->read(reinterpret_cast<char*>(frame.image.bits()),
                           static_cast<qint64>(header.imageSize)) !=
        static_cast<qint64>(header.imageSize))
    {
        qWarning() << "Failed to read image:" << segment.file->errorString();
        return std::nullopt;
    }

    if (header.hasPresentationTimestamp)
        frame.presentationTimestampNs = header.presentationTimestampNs;
    else
        frame.presentationTimestampNs.reset();

    return frame;
}

void VideoFrameSpool::release(const FrameLocation& location)
{
    assert(location.segment >= 0);
    assert(static_cast<std::size_t>(location.segment) < segments_.size());

    auto& segment = segments_[location.segment];

    assert(segment.refCount > 0);

    --segment.refCount;

    if (segment.refCount == 0)
    {
        if (segment.file->resize(0))
        {
            segment.writeOffset = 0;
        }
        else
        {
            qWarning() << "Failed to truncate spool segment:" << segment.file->fileName() << ":"
                       << segment.file->errorString();
        }
    }
}

} // namespace fluvel