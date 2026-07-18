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
    const auto frameBytes = frameSize(frame.image);

    if (queuedBytes_ + frameBytes > settings_.maxRamUsage)
    {
        if (queuedDiskBytes_ + frameBytes > settings_.maxDiskUsage)
        {
            switch (settings_.overflowPolicy)
            {
                case BufferOverflowPolicy::StopRecording:
                    return PushStatus::BufferLimitExceeded;

                case BufferOverflowPolicy::Circular:
                    // TODO
                    return PushStatus::BufferLimitExceeded;
            }
        }

        auto location = spool_.write(frame);

        if (!location)
            return PushStatus::BufferLimitExceeded;

        BufferedFrame bufferedFrame;

        bufferedFrame.storage = StorageType::Disk;
        bufferedFrame.location = *location;

        queue_.enqueue(std::move(bufferedFrame));
        queuedDiskBytes_ += frameBytes;

        if (!usingTemporaryStorage_)
        {
            usingTemporaryStorage_ = true;
            return PushStatus::TemporaryStorageActivated;
        }

        return PushStatus::Success;
    }

    VideoFrame queuedFrame = frame;
    queuedFrame.image = frame.image.copy();

    BufferedFrame bufferedFrame;
    bufferedFrame.storage = StorageType::Memory;
    bufferedFrame.frame = std::move(queuedFrame);

    queue_.enqueue(std::move(bufferedFrame));
    queuedBytes_ += frameBytes;

    return PushStatus::Success;
}

std::optional<VideoFrame> VideoFrameBuffer::pop()
{
    if (queue_.isEmpty())
        return std::nullopt;

    auto bufferedFrame = queue_.dequeue();

    switch (bufferedFrame.storage)
    {
        case StorageType::Memory:
            queuedBytes_ -= frameSize(bufferedFrame.frame.image);
            return std::move(bufferedFrame.frame);

        case StorageType::Disk:
        {
            auto frame = spool_.read(bufferedFrame.location);

            if (frame)
                queuedDiskBytes_ -= frameSize(frame->image);

            return frame;
        }
    }

    return std::nullopt;
}

void VideoFrameBuffer::removeTemporaryStorage()
{
    queue_.clear();

    if (!spool_.remove())
        qWarning() << "Failed to remove temporary storage.";

    queuedBytes_ = 0;
    queuedDiskBytes_ = 0;
    usingTemporaryStorage_ = false;
}

void VideoFrameBuffer::clear()
{
    queue_.clear();

    if (!spool_.reset())
        qWarning() << "Failed to clear temporary storage.";

    queuedBytes_ = 0;
    queuedDiskBytes_ = 0;
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

uint64_t VideoFrameBuffer::queuedBytes() const
{
    return queuedBytes_ + queuedDiskBytes_;
}

uint64_t VideoFrameBuffer::queuedMemoryBytes() const
{
    return queuedBytes_;
}

uint64_t VideoFrameBuffer::queuedDiskBytes() const
{
    return queuedDiskBytes_;
}

uint64_t VideoFrameBuffer::frameSize(const QImage& image)
{
    return static_cast<uint64_t>(image.sizeInBytes());
}

void VideoFrameBuffer::setSettings(const RecordingBufferSettings& settings)
{
    settings_ = settings;
}

} // namespace fluvel