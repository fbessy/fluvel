// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_video_exporter.hpp"

#include "video_export_settings.hpp"

#include <QImage>

namespace fluvel
{

struct FFmpegVideoExporter::Context
{
    //
    // TODO
    //
    // AVFormatContext* formatContext = nullptr;
    // AVCodecContext* codecContext = nullptr;
    // AVStream* stream = nullptr;
    // AVFrame* frame = nullptr;
    // AVPacket* packet = nullptr;
    // SwsContext* swsContext = nullptr;
    //
};

FFmpegVideoExporter::FFmpegVideoExporter()
    : context_(std::make_unique<Context>())
{
}

FFmpegVideoExporter::~FFmpegVideoExporter()
{
    release();
}

bool FFmpegVideoExporter::open(const VideoExportSettings& settings)
{
    if (isOpen_)
        return false;

    settings_ = settings;

    applyExportProfile(settings_);

    if (!initializeContainer(settings_))
        return false;

    if (!initializeCodec(settings_))
    {
        release();
        return false;
    }

    if (!initializeStream())
    {
        release();
        return false;
    }

    if (!openOutputFile())
    {
        release();
        return false;
    }

    if (!writeHeader())
    {
        release();
        return false;
    }

    isOpen_ = true;

    return true;
}

bool FFmpegVideoExporter::addFrame(const QImage& image)
{
    if (!isOpen_)
        return false;

    Q_UNUSED(image);

    //
    // TODO
    //
    // 1. Convert QImage to AVFrame
    // 2. RGB -> YUV
    // 3. avcodec_send_frame()
    // 4. avcodec_receive_packet()
    // 5. av_interleaved_write_frame()
    //

    return true;
}

bool FFmpegVideoExporter::close()
{
    if (!isOpen_)
        return false;

    flushEncoder();

    writeTrailer();

    release();

    isOpen_ = false;

    return true;
}

bool FFmpegVideoExporter::isOpen() const
{
    return isOpen_;
}

void FFmpegVideoExporter::applyExportProfile(VideoExportSettings& settings) const
{
    switch (settings.profile)
    {
        case ExportProfile::Archive:

            settings.codec = VideoCodec::FFV1;
            settings.container = VideoContainer::Matroska;
            break;

        case ExportProfile::Compatible:

            settings.codec = VideoCodec::H264;
            settings.container = VideoContainer::Mp4;
            break;

        case ExportProfile::Balanced:

            settings.codec = VideoCodec::H265;
            settings.container = VideoContainer::Mp4;
            break;

        case ExportProfile::Efficient:

            settings.codec = VideoCodec::AV1;
            settings.container = VideoContainer::Mp4;
            break;

        case ExportProfile::Custom:

            break;
    }
}

bool FFmpegVideoExporter::initializeContainer(const VideoExportSettings& settings)
{
    Q_UNUSED(settings);

    //
    // TODO
    //
    // av_guess_format(...)
    // avformat_alloc_output_context2(...)
    //

    return true;
}

bool FFmpegVideoExporter::initializeCodec(const VideoExportSettings& settings)
{
    Q_UNUSED(settings);

    //
    // TODO
    //
    // avcodec_find_encoder(...)
    // avcodec_alloc_context3(...)
    // configure codec
    // avcodec_open2(...)
    //

    return true;
}

bool FFmpegVideoExporter::initializeStream()
{
    //
    // TODO
    //
    // avformat_new_stream(...)
    // avcodec_parameters_from_context(...)
    //

    return true;
}

bool FFmpegVideoExporter::openOutputFile()
{
    //
    // TODO
    //
    // avio_open(...)
    //

    return true;
}

bool FFmpegVideoExporter::writeHeader()
{
    //
    // TODO
    //
    // avformat_write_header(...)
    //

    return true;
}

bool FFmpegVideoExporter::flushEncoder()
{
    //
    // TODO
    //
    // avcodec_send_frame(nullptr)
    // receive remaining packets
    //

    return true;
}

bool FFmpegVideoExporter::writeTrailer()
{
    //
    // TODO
    //
    // av_write_trailer(...)
    //

    return true;
}

void FFmpegVideoExporter::release()
{
    //
    // TODO
    //
    // sws_freeContext(...)
    // av_frame_free(...)
    // av_packet_free(...)
    // avcodec_free_context(...)
    // avio_closep(...)
    // avformat_free_context(...)
    //
}

} // namespace fluvel