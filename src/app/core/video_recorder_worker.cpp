#include "video_recorder_worker.hpp"

#include "frame_clock.hpp"
#include "video_exporter.hpp"

namespace fluvel
{

VideoRecorderWorker::VideoRecorderWorker()
{
}

VideoRecorderWorker::~VideoRecorderWorker()
{
    stop();

    if (workerThread_.joinable())
        workerThread_.join();
}

void VideoRecorderWorker::start(const VideoExportSettings& settings)
{
    if (state_ != RecorderState::Stopped)
        return;

    if (workerThread_.joinable())
        workerThread_.join();

    if (!exporter_.open(settings))
    {
        emit errorOccurred(tr("Failed to start video recording."));
        return;
    }

    {
        QMutexLocker locker(&mutex_);
        resetSession();
    }

    state_ = RecorderState::Recording;

    workerThread_ = std::thread(&VideoRecorderWorker::processQueue, this);

    emit stateChanged(RecorderState::Recording);
}

void VideoRecorderWorker::stop()
{
    {
        QMutexLocker locker(&mutex_);

        if (state_ != RecorderState::Recording)
            return;

        state_ = RecorderState::Draining;
    }

    condition_.wakeOne();

    emit stateChanged(RecorderState::Draining);
}

bool VideoRecorderWorker::isRecording() const
{
    return state_ != RecorderState::Stopped;
}

bool VideoRecorderWorker::isAcceptingFrames() const
{
    return state_ == RecorderState::Recording;
}

void VideoRecorderWorker::addFrame(const VideoFrame& frame)
{
    if (state_ != RecorderState::Recording)
        return;

    enqueue(frame);
}

void VideoRecorderWorker::enqueue(const VideoFrame& frame)
{
    const std::size_t bytes = frameSize(frame.image);

    EnqueueStatus status = EnqueueStatus::Success;

    {
        QMutexLocker locker(&mutex_);

        if (state_ != RecorderState::Recording)
            return;

        if (queuedBytes_ + bytes > kMaxMemoryBytes)
        {
            state_ = RecorderState::Draining;
            status = EnqueueStatus::MemoryLimitExceeded;
        }
        else
        {
            if (!memoryWarningEmitted_ && queuedBytes_ + bytes > kWarningMemoryBytes)
            {
                memoryWarningEmitted_ = true;
                status = EnqueueStatus::MemoryWarning;
            }

            queue_.enqueue(frame);
            queuedBytes_ += bytes;
            ++inputFrameCount_;
        }
    }

    condition_.wakeOne();

    switch (status)
    {
        case EnqueueStatus::Success:
            break;

        case EnqueueStatus::MemoryWarning:
            emit warningOccurred(tr("Video recorder queue exceeds the recommended memory usage."));
            break;

        case EnqueueStatus::MemoryLimitExceeded:
            emit stateChanged(RecorderState::Draining);
            emit errorOccurred(tr("Video recording stopped because the encoder cannot keep up "
                                  "with the incoming frame rate."));
            break;
    }
}

void VideoRecorderWorker::processQueue()
{
    for (;;)
    {
        VideoFrame frame;

        {
            QMutexLocker locker(&mutex_);

            while (queue_.isEmpty() && state_ == RecorderState::Recording)
                condition_.wait(&mutex_);

            if (queue_.isEmpty())
                break;

            frame = queue_.dequeue();
            queuedBytes_ -= frameSize(frame.image);
        }

        if (!exporter_.addFrame(frame))
        {
            {
                QMutexLocker locker(&mutex_);

                queue_.clear();
                queuedBytes_ = 0;
                state_ = RecorderState::Draining;
            }

            emit stateChanged(RecorderState::Draining);
            emit errorOccurred(tr("Failed to encode video frame."));

            break;
        }

        {
            QMutexLocker locker(&mutex_);
            ++encodedFrameCount_;
        }

        updateStats();
    }

    const bool success = exporter_.close();

    state_ = RecorderState::Stopped;

    if (!success)
    {
        emit errorOccurred(tr("Failed to finalize video recording."));
    }
    else
    {
        emit recordingFinalized();
    }

    emit stateChanged(RecorderState::Stopped);
}

std::size_t VideoRecorderWorker::frameSize(const QImage& image)
{
    return static_cast<std::size_t>(image.sizeInBytes());
}

void VideoRecorderWorker::resetSession()
{
    memoryWarningEmitted_ = false;

    queuedBytes_ = 0;
    queue_.clear();

    inputFrameCount_ = 0;
    encodedFrameCount_ = 0;

    statsTimestampNs_ = FrameClock::nowNs();
}

void VideoRecorderWorker::updateStats()
{
    const int64_t nowNs = FrameClock::nowNs();
    const int64_t elapsedNs = nowNs - statsTimestampNs_;

    if (elapsedNs < kStatsIntervalNs)
        return;

    RecorderStats stats;

    {
        QMutexLocker locker(&mutex_);

        const double elapsedSec = static_cast<double>(elapsedNs) * 1e-9;

        stats.queuedFrames = static_cast<std::size_t>(queue_.size());
        stats.queuedBytes = queuedBytes_;
        stats.inputFps = static_cast<double>(inputFrameCount_) / elapsedSec;
        stats.encodingFps = static_cast<double>(encodedFrameCount_) / elapsedSec;

        inputFrameCount_ = 0;
        encodedFrameCount_ = 0;
        statsTimestampNs_ = nowNs;
    }

    emit statsChanged(stats);
}

} // namespace fluvel