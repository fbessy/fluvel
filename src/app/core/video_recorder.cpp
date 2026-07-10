#include "video_recorder.hpp"

#include "video_exporter.hpp"

namespace fluvel
{

VideoRecorder::VideoRecorder()
{
}

VideoRecorder::~VideoRecorder()
{
    stop();
}

bool VideoRecorder::start(const VideoExportSettings& settings)
{
    if (recording_)
        return false;

    if (!exporter_.open(settings))
        return false;

    recording_ = true;

    return true;
}

bool VideoRecorder::stop()
{
    if (!recording_)
        return true;

    recording_ = false;

    return exporter_.close();
}

bool VideoRecorder::isRecording() const
{
    return recording_;
}

bool VideoRecorder::addFrame(const QImage& image)
{
    if (!recording_)
        return false;

    return exporter_.addFrame(image);
}

} // namespace fluvel