// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_frame_buffer.hpp"
#include "recording_types.hpp"

namespace fluvel
{

VideoFrameBuffer::VideoFrameBuffer()
{
    if (!spool_.open())
        qWarning() << "Failed to initialize video frame spool.";
}

VideoFrameBuffer::PushStatus VideoFrameBuffer::push(const VideoFrame& frame)
{
    switch (settings_.overflowPolicy)
    {
        case BufferOverflowPolicy::StopRecording:
            return pushStopRecording(frame);

        case BufferOverflowPolicy::DiscardOldest:
            return pushDiscardOldest(frame);
    }

    assert(false);
    return PushStatus::BufferLimitExceeded;
}

VideoFrameBuffer::PushStatus VideoFrameBuffer::pushStopRecording(const VideoFrame& frame)
{
    const auto frameBytes = frameSize(frame.image);

    // push to RAM in priority if possible
    if (queuedBytes_ + frameBytes <= settings_.maxRamUsage)
        return pushToMemory(frame, frameBytes);

    if (queuedDiskBytes_ + frameBytes <= settings_.maxDiskUsage)
        return pushToDisk(frame, frameBytes);

    return PushStatus::BufferLimitExceeded;
}

VideoFrameBuffer::PushStatus VideoFrameBuffer::pushDiscardOldest(const VideoFrame& frame)
{
    const auto frameBytes = frameSize(frame.image);

    if (queuedBytes_ + frameBytes <= settings_.maxRamUsage)
        return pushToMemory(frame, frameBytes);

    if (queuedDiskBytes_ + frameBytes <= settings_.maxDiskUsage)
        return pushToDisk(frame, frameBytes);

    while (queuedDiskBytes_ + frameBytes > settings_.maxDiskUsage)
    {
        if (queue_.isEmpty())
            return PushStatus::BufferLimitExceeded;

        auto oldest = queue_.dequeue();

        switch (oldest.storage)
        {
            case StorageType::Memory:
                queuedBytes_ -= oldest.sizeBytes;
                break;

            case StorageType::Disk:
                queuedDiskBytes_ -= oldest.sizeBytes;
                break;
        }
    }

    return pushToDisk(frame, frameBytes);
}

VideoFrameBuffer::PushStatus VideoFrameBuffer::pushToMemory(const VideoFrame& frame,
                                                            qsizetype frameBytes)
{
    VideoFrame queuedFrame = frame;
    queuedFrame.image = frame.image.copy();

    BufferedFrame bufferedFrame;
    bufferedFrame.storage = StorageType::Memory;
    bufferedFrame.sizeBytes = frameBytes;
    bufferedFrame.presentationTimestampNs = frame.presentationTimestampNs;
    bufferedFrame.frame = std::move(queuedFrame);

    queue_.enqueue(std::move(bufferedFrame));
    queuedBytes_ += frameBytes;

    return PushStatus::Success;
}

VideoFrameBuffer::PushStatus VideoFrameBuffer::pushToDisk(const VideoFrame& frame,
                                                          qsizetype frameBytes)
{
    auto location = spool_.write(frame);

    if (!location)
        return PushStatus::BufferLimitExceeded;

    BufferedFrame bufferedFrame;

    bufferedFrame.storage = StorageType::Disk;
    bufferedFrame.sizeBytes = frameBytes;
    bufferedFrame.presentationTimestampNs = frame.presentationTimestampNs;
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

void VideoFrameBuffer::reset()
{
    queue_.clear();

    if (!spool_.reset())
        qWarning() << "Failed to reset temporary spool.";

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

void VideoFrameBuffer::fillStats(RecorderStats& stats) const
{
    stats.queuedFrames = static_cast<std::size_t>(queue_.size());
    stats.queuedMemoryBytes = queuedBytes_;
    stats.queuedDiskBytes = queuedDiskBytes_;

    stats.retainedDuration = std::chrono::milliseconds::zero();

    // Duration currently retained by the recorder.
    if (stats.queuedFrames >= 2)
    {
        const auto& oldest = queue_.head();
        const auto& newest = queue_.back();

        if (oldest.presentationTimestampNs && newest.presentationTimestampNs)
        {
            const auto durationNs =
                *newest.presentationTimestampNs - *oldest.presentationTimestampNs;

            if (durationNs > 0)
            {
                stats.retainedDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::nanoseconds(durationNs));
            }
        }
    }

    // Estimated maximum retained duration.
    stats.estimatedMaxRetainedDuration.reset();

    const uint64_t currentBufferedBytes = stats.queuedMemoryBytes + stats.queuedDiskBytes;

    if (currentBufferedBytes > 0 && stats.retainedDuration > std::chrono::milliseconds::zero())
    {
        const uint64_t maxBufferedBytes = settings_.maxRamUsage + settings_.maxDiskUsage;

        const double capacityRatio =
            static_cast<double>(maxBufferedBytes) / static_cast<double>(currentBufferedBytes);

        const auto estimatedMs =
            static_cast<int64_t>(stats.retainedDuration.count() * capacityRatio);

        stats.estimatedMaxRetainedDuration = std::chrono::milliseconds(estimatedMs);
    }
}

} // namespace fluvel