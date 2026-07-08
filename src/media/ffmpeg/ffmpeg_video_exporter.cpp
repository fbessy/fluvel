// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_video_exporter.hpp"

#include "video_export_settings.hpp"

#include <QImage>

extern "C"
{
#include <libavformat/avformat.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
}

namespace fluvel
{

struct FFmpegVideoExporter::Context
{
    AVFormatContext* formatContext{nullptr};

    const AVCodec* codec{nullptr};
    AVCodecContext* codecContext{nullptr};

    AVStream* stream{nullptr};

    AVFrame* frame{nullptr};
    AVPacket* packet{nullptr};
    SwsContext* swsContext{nullptr};

    int64_t nextPts{0};
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

    if (!allocateFrame())
    {
        release();
        return false;
    }

    if (!allocatePacket())
    {
        release();
        return false;
    }

    if (!initializeScaler())
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

    context_->nextPts = 0;

    isOpen_ = true;

    return true;
}

bool FFmpegVideoExporter::addFrame(const QImage& image)
{
    if (!isOpen_)
        return false;

    if (!fillFrame(image))
        return false;

    return encodeFrame();
}

bool FFmpegVideoExporter::close()
{
    if (!isOpen_)
        return false;

    if (!flushEncoder())
    {
        release();
        isOpen_ = false;
        return false;
    }

    if (!writeTrailer())
    {
        release();
        isOpen_ = false;
        return false;
    }

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
            // No override.
            break;
    }
}

bool FFmpegVideoExporter::initializeContainer(const VideoExportSettings& settings)
{
    const char* format = nullptr;

    switch (settings.container)
    {
        case VideoContainer::Matroska:
            format = "matroska";
            break;

        case VideoContainer::Mp4:
            format = "mp4";
            break;

        case VideoContainer::Avi:
            format = "avi";
            break;

        case VideoContainer::Mov:
            format = "mov";
            break;

        case VideoContainer::WebM:
            format = "webm";
            break;
    }

    const int ret = avformat_alloc_output_context2(&context_->formatContext, nullptr, format,
                                                   settings.filename.toUtf8().constData());

    return ret >= 0 && context_->formatContext != nullptr;
}

bool FFmpegVideoExporter::initializeCodec(const VideoExportSettings& settings)
{
    AVCodecID codecId = AV_CODEC_ID_NONE;

    switch (settings.codec)
    {
        case VideoCodec::FFV1:
            codecId = AV_CODEC_ID_FFV1;
            break;

        case VideoCodec::H264:
            codecId = AV_CODEC_ID_H264;
            break;

        case VideoCodec::H265:
            codecId = AV_CODEC_ID_HEVC;
            break;

        case VideoCodec::MPEG4Part2:
            codecId = AV_CODEC_ID_MPEG4;
            break;

        case VideoCodec::VP9:
            codecId = AV_CODEC_ID_VP9;
            break;

        case VideoCodec::AV1:
            codecId = AV_CODEC_ID_AV1;
            break;

        default:
            Q_UNREACHABLE();
            return false;
    }

    context_->codec = avcodec_find_encoder(codecId);

    if (context_->codec == nullptr)
        return false;

    context_->codecContext = avcodec_alloc_context3(context_->codec);

    if (context_->codecContext == nullptr)
        return false;

    auto* c = context_->codecContext;

    c->codec_id = codecId;

    c->codec_type = AVMEDIA_TYPE_VIDEO;

    c->width = settings.width;

    c->height = settings.height;

    c->time_base = AVRational{1, settings.frameRate};

    c->framerate = AVRational{settings.frameRate, 1};

    //
    // TODO:
    // Select the best pixel format supported by the encoder.
    //
    switch (settings.codec)
    {
        case VideoCodec::FFV1:
            c->pix_fmt = AV_PIX_FMT_BGR0;
            break;

        default:
            c->pix_fmt = AV_PIX_FMT_YUV420P;
    }

    if (context_->formatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    const int ret = avcodec_open2(c, context_->codec, nullptr);

    return ret >= 0;
}

bool FFmpegVideoExporter::initializeStream()
{
    context_->stream = avformat_new_stream(context_->formatContext, nullptr);

    if (context_->stream == nullptr)
        return false;

    context_->stream->time_base = context_->codecContext->time_base;

    const int ret =
        avcodec_parameters_from_context(context_->stream->codecpar, context_->codecContext);

    return ret >= 0;
}

bool FFmpegVideoExporter::openOutputFile()
{
    if (context_->formatContext == nullptr)
        return false;

    if (context_->formatContext->oformat->flags & AVFMT_NOFILE)
        return true;

    const QByteArray filename = settings_.filename.toUtf8();

    const int ret = avio_open(&context_->formatContext->pb, filename.constData(), AVIO_FLAG_WRITE);

    return ret >= 0;
}

bool FFmpegVideoExporter::writeHeader()
{
    const int ret = avformat_write_header(context_->formatContext, nullptr);

    return ret >= 0;
}

bool FFmpegVideoExporter::flushEncoder()
{
    //
    // Signal end-of-stream to the encoder.
    //
    const int ret = avcodec_send_frame(context_->codecContext, nullptr);

    if (ret < 0)
        return false;

    return receivePackets();
}

bool FFmpegVideoExporter::writeTrailer()
{
    return av_write_trailer(context_->formatContext) >= 0;
}

void FFmpegVideoExporter::release()
{
    //
    // Most FFmpeg deallocation helpers taking a pointer-to-pointer automatically
    // reset the pointer to nullptr.
    //

    if (context_->swsContext != nullptr)
        sws_freeContext(context_->swsContext);

    if (context_->frame != nullptr)
        av_frame_free(&context_->frame);

    if (context_->packet != nullptr)
        av_packet_free(&context_->packet);

    if (context_->codecContext != nullptr)
        avcodec_free_context(&context_->codecContext);

    if (context_->formatContext != nullptr)
    {
        if (!(context_->formatContext->oformat->flags & AVFMT_NOFILE))
        {
            avio_closep(&context_->formatContext->pb);
        }

        avformat_free_context(context_->formatContext);
        context_->formatContext = nullptr;
    }

    //
    // Runtime state
    //
    context_->codec = nullptr;
    context_->stream = nullptr;
    context_->nextPts = 0;
}

bool FFmpegVideoExporter::allocateFrame()
{
    context_->frame = av_frame_alloc();

    if (context_->frame == nullptr)
        return false;

    auto* frame = context_->frame;
    auto* codec = context_->codecContext;

    frame->format = codec->pix_fmt;
    frame->width = codec->width;
    frame->height = codec->height;

    const int ret = av_frame_get_buffer(frame, 32);

    return ret >= 0;
}

bool FFmpegVideoExporter::allocatePacket()
{
    context_->packet = av_packet_alloc();

    return context_->packet != nullptr;
}

bool FFmpegVideoExporter::initializeScaler()
{
    //
    // FFV1 in BGR0 does not require any conversion.
    //
    if (settings_.codec == VideoCodec::FFV1)
        return true;

    auto* c = context_->codecContext;

    context_->swsContext = sws_getContext(c->width, c->height, AV_PIX_FMT_BGRA, c->width, c->height,
                                          c->pix_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);

    return context_->swsContext != nullptr;
}

bool FFmpegVideoExporter::makeFrameWritable()
{
    return av_frame_make_writable(context_->frame) >= 0;
}

bool FFmpegVideoExporter::receivePackets()
{
    //
    // Retrieve all available encoded packets.
    //
    while (true)
    {
        const int receiveRet = avcodec_receive_packet(context_->codecContext, context_->packet);

        if (receiveRet == AVERROR(EAGAIN) || receiveRet == AVERROR_EOF)
            break;

        if (receiveRet < 0)
            return false;

        av_packet_rescale_ts(context_->packet, context_->codecContext->time_base,
                             context_->stream->time_base);

        context_->packet->stream_index = context_->stream->index;

        const int writeRet = av_interleaved_write_frame(context_->formatContext, context_->packet);

        av_packet_unref(context_->packet);

        if (writeRet < 0)
            return false;
    }

    return true;
}

bool FFmpegVideoExporter::fillFrame(const QImage& image)
{
    if (!makeFrameWritable())
        return false;

    switch (context_->codecContext->pix_fmt)
    {
        case AV_PIX_FMT_BGR0:
            return fillFrameBgr0(image);

        case AV_PIX_FMT_YUV420P:
            return fillFrameYuv420(image);

        default:
            return false;
    }
}

bool FFmpegVideoExporter::fillFrameBgr0(const QImage& image)
{
    const QImage* src = &image;

    QImage converted;

    if (image.format() != QImage::Format_ARGB32)
    {
        converted = image.convertToFormat(QImage::Format_ARGB32);
        src = &converted;
    }

    auto* frame = context_->frame;

    constexpr int kBytesPerPixel = 4;
    const int bytesPerRow = src->width() * kBytesPerPixel;

    //
    // QImage::Format_ARGB32 is stored as BGRA in memory, which is compatible
    // with AV_PIX_FMT_BGR0.
    //
    // Copy the image one scanline at a time to correctly handle different source
    // and destination strides. QImage::constScanLine() accounts for the source
    // stride, while AVFrame::linesize specifies the destination stride.
    //
    for (int y = 0; y < src->height(); ++y)
    {
        std::memcpy(frame->data[0] + y * frame->linesize[0], src->constScanLine(y),
                    static_cast<std::size_t>(bytesPerRow));
    }

    return true;
}

bool FFmpegVideoExporter::fillFrameYuv420(const QImage& image)
{
    QImage src = image;

    if (src.format() != QImage::Format_ARGB32)
    {
        src = src.convertToFormat(QImage::Format_ARGB32);
    }

    const uint8_t* srcData[1] = {src.constBits()};

    const int srcStride[1] = {src.bytesPerLine()};

    sws_scale(context_->swsContext, srcData, srcStride, 0, src.height(), context_->frame->data,
              context_->frame->linesize);

    return true;
}

bool FFmpegVideoExporter::encodeFrame()
{
    auto* frame = context_->frame;

    frame->pts = context_->nextPts++;

    const int ret = avcodec_send_frame(context_->codecContext, frame);

    if (ret < 0)
        return false;

    return receivePackets();
}

} // namespace fluvel