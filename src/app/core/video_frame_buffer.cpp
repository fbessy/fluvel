// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_frame_buffer.hpp"

namespace fluvel
{

VideoFrameBuffer::VideoFrameBuffer()
{
    if (!spool_.open())
        qWarning() << "Failed to initialize video frame spool.";
}

VideoFrameBuffer::PushStatus VideoFrameBuffer::push(const VideoFrame& frame)
{
    VideoFrame queuedFrame = frame;
    queuedFrame.image = frame.image.copy();

    const std::size_t bytes = frameSize(queuedFrame.image);

    if (queuedBytes_ + bytes > settings_.maxRamUsage)
    {
        auto location = spool_.write(queuedFrame);

        if (!location)
            return PushStatus::BufferLimitExceeded;

        BufferedFrame bufferedFrame;

        bufferedFrame.storage = StorageType::Disk;
        bufferedFrame.location = *location;

        queue_.enqueue(std::move(bufferedFrame));

        if (!usingTemporaryStorage_)
        {
            usingTemporaryStorage_ = true;
            return PushStatus::TemporaryStorageActivated;
        }

        return PushStatus::Success;
    }

    BufferedFrame bufferedFrame;

    bufferedFrame.storage = StorageType::Memory;
    bufferedFrame.frame = std::move(queuedFrame);

    queue_.enqueue(std::move(bufferedFrame));
    queuedBytes_ += bytes;

    return PushStatus::Success;
}

std::optional<VideoFrame> VideoFrameBuffer::pop()
{
    if (queue_.isEmpty())
        return std::nullopt;

    auto bufferedFrame = queue_.dequeue();

    queuedBytes_ -= frameSize(bufferedFrame.frame.image);

    switch (bufferedFrame.storage)
    {
        case StorageType::Memory:
            return std::move(bufferedFrame.frame);

        case StorageType::Disk:
            return spool_.read(bufferedFrame.location);
    }

    return std::nullopt;
}

void VideoFrameBuffer::clear()
{
    queue_.clear();

    if (!spool_.clear())
        qWarning() << "Failed to clear temporary storage.";

    queuedBytes_ = 0;
    usingTemporaryStorage_ = false;
}

bool VideoFrameBuffer::empty() const
{
    return queue_.isEmpty();
}

std::size_t VideoFrameBuffer::queuedFrames() const
{
    return static_cast<std::size_t>(queue_.size());
}

std::size_t VideoFrameBuffer::queuedBytes() const
{
    return queuedBytes_;
}

std::size_t VideoFrameBuffer::frameSize(const QImage& image)
{
    return static_cast<std::size_t>(image.sizeInBytes());
}

} // namespace fluvel