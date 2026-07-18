#include "video_recorder_worker.hpp"

#include "frame_clock.hpp"
#include "video_exporter.hpp"

#include <QThread>

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

        frameBuffer_.setSettings(settings.bufferSettings);
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
    VideoFrameBuffer::PushStatus status;

    {
        QMutexLocker locker(&mutex_);

        if (state_ != RecorderState::Recording)
            return;

        status = frameBuffer_.push(frame);

        if (status != VideoFrameBuffer::PushStatus::BufferLimitExceeded)
            ++inputFrameCount_;

        if (status == VideoFrameBuffer::PushStatus::BufferLimitExceeded)
            state_ = RecorderState::Draining;
    }

    condition_.wakeOne();

    switch (status)
    {
        case VideoFrameBuffer::PushStatus::Success:
            break;

        case VideoFrameBuffer::PushStatus::TemporaryStorageActivated:
            break;

        case VideoFrameBuffer::PushStatus::BufferLimitExceeded:
            emit stateChanged(RecorderState::Draining);

            emit errorOccurred(tr("Video recording stopped because the recording buffer is full."));
            break;
    }
}

void VideoRecorderWorker::processQueue()
{
    constexpr int kMaxEncodingAttempts = 3;

    for (;;)
    {
        std::optional<VideoFrame> frame;

        {
            QMutexLocker locker(&mutex_);

            for (;;)
            {
                frame = frameBuffer_.pop();

                if (frame)
                    break;

                if (state_ != RecorderState::Recording)
                    break;

                condition_.wait(&mutex_);
            }

            if (!frame)
                break;
        }

        bool success = false;

        for (int attempt = 0; attempt < kMaxEncodingAttempts; ++attempt)
        {
            if (exporter_.addFrame(*frame))
            {
                success = true;
                break;
            }

            QThread::msleep(50); // Optionnel
        }

        if (!success)
        {
            {
                QMutexLocker locker(&mutex_);
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

    {
        QMutexLocker locker(&mutex_);

        frameBuffer_.removeTemporaryStorage();
    }

    state_ = RecorderState::Stopped;

    if (!success)
        emit errorOccurred(tr("Failed to finalize video recording."));
    else
        emit recordingFinalized();

    emit stateChanged(RecorderState::Stopped);
}

void VideoRecorderWorker::resetSession()
{
    frameBuffer_.clear();

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

        stats.queuedFrames = frameBuffer_.queuedFrames();
        stats.queuedMemoryBytes = frameBuffer_.queuedMemoryBytes();
        stats.queuedDiskBytes = frameBuffer_.queuedDiskBytes();
        stats.inputFps = static_cast<double>(inputFrameCount_) / elapsedSec;
        stats.encodingFps = static_cast<double>(encodedFrameCount_) / elapsedSec;

        inputFrameCount_ = 0;
        encodedFrameCount_ = 0;
        statsTimestampNs_ = nowNs;
    }

    emit statsChanged(stats);
}

} // namespace fluvel