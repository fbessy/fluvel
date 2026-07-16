// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_frame_buffer.hpp"

namespace fluvel
{

VideoFrameBuffer::PushStatus VideoFrameBuffer::push(const VideoFrame& frame)
{
    VideoFrame queuedFrame = frame;
    queuedFrame.image = frame.image.copy();

    const std::size_t bytes = frameSize(queuedFrame.image);

    if (queuedBytes_ + bytes > kMaxMemoryBytes)
        return PushStatus::MemoryLimitExceeded;

    bool memoryWarning = false;

    if (!memoryWarningEmitted_ && queuedBytes_ + bytes > kWarningMemoryBytes)
    {
        memoryWarningEmitted_ = true;
        memoryWarning = true;
    }

    queue_.enqueue(std::move(queuedFrame));
    queuedBytes_ += bytes;

    return memoryWarning ? PushStatus::MemoryWarning : PushStatus::Success;
}

std::optional<VideoFrame> VideoFrameBuffer::pop()
{
    if (queue_.isEmpty())
        return std::nullopt;

    VideoFrame frame = std::move(queue_.head());

    queue_.dequeue();

    queuedBytes_ -= frameSize(frame.image);

    return frame;
}

void VideoFrameBuffer::clear()
{
    queue_.clear();

    queuedBytes_ = 0;
    memoryWarningEmitted_ = false;
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