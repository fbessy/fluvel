// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_video_exporter.hpp"

#include "ffmpeg_codec_utils.hpp"
#include "ffmpeg_utils.hpp"
#include "video_export_settings.hpp"
#include "video_exporter_utils.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QImage>

#include <cassert>

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
    //
    // FFmpeg objects
    //
    AVFormatContext* formatContext{nullptr};

    const AVCodec* codec{nullptr};
    AVCodecContext* codecContext{nullptr};

    AVStream* stream{nullptr};

    AVFrame* frame{nullptr};
    AVPacket* packet{nullptr};

    SwsContext* swsContext{nullptr};

    //
    // Encoding state
    //
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
    if (state_ != ExportState::Closed)
        return false;

    //
    // Store the effective export settings.
    //
    settings_ = settings;

    applyExportProfile(settings_);

    if (!hasExpectedExtension(settings_.filename, settings_.container))
    {
        qWarning() << "Filename extension" << QFileInfo(settings_.filename).suffix()
                   << "does not match the selected" << exporter_utils::toString(settings_.container)
                   << "container.";
    }

    state_ = ExportState::WaitingForFirstFrame;

    return true;
}

bool FFmpegVideoExporter::close()
{
    if (state_ == ExportState::Closed)
        return false;

    bool success = true;

    if (state_ == ExportState::Recording)
    {
        success = flushEncoder() && writeTrailer();
    }

    release();
    state_ = ExportState::Closed;

    return success;
}

bool FFmpegVideoExporter::isRecording() const
{
    return state_ == ExportState::Recording;
}

bool FFmpegVideoExporter::addFrame(const QImage& image)
{
    if (image.isNull() || image.size().isEmpty())
        return false;

    if (state_ == ExportState::WaitingForFirstFrame)
    {
        if (!initializeFromFirstFrame(image))
            return false;
    }

    if (state_ != ExportState::Recording)
        return false;

    if (image.size() != frameSize_)
    {
        qWarning() << "Frame size" << image.size() << "does not match initial frame size"
                   << frameSize_;

        return false;
    }

    if (!fillFrame(image) || !encodeFrame())
    {
        release();
        state_ = ExportState::Closed;
        return false;
    }

    return true;
}

bool FFmpegVideoExporter::initializeFromFirstFrame(const QImage& firstFrame)
{
    assert(!firstFrame.isNull());
    assert(!firstFrame.size().isEmpty());

    if (state_ != ExportState::WaitingForFirstFrame)
        return false;

    frameSize_ = firstFrame.size();

    if (!initializeContainer(settings_) || !initializeCodec(settings_) || !initializeStream() ||
        !allocateFrame() || !allocatePacket() || !initializeScaler() || !openOutputFile() ||
        !writeHeader())
    {
        release();
        state_ = ExportState::Closed;
        return false;
    }

    context_->nextPts = 0;

    state_ = ExportState::Recording;

    return true;
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

            if (settings.codec == VideoCodec::FFV1 &&
                settings.container != VideoContainer::Matroska)
            {
                qWarning() << "FFV1 is typically stored in a Matroska container.";
            }

            if (settings.codec == VideoCodec::VP9 && settings.container != VideoContainer::WebM)
            {
                qWarning() << "VP9 is typically stored in a WebM container.";
            }

            if ((settings.codec == VideoCodec::H264 || settings.codec == VideoCodec::H265) &&
                settings.container != VideoContainer::Mp4)
            {
                qInfo() << exporter_utils::toString(settings.codec)
                        << "is commonly stored in an MP4 container.";
            }

            // No override.
            break;
    }
}

bool FFmpegVideoExporter::hasExpectedExtension(const QString& filename, VideoContainer container)
{
    const QString extension = QFileInfo(filename).suffix().toLower();

    switch (container)
    {
        case VideoContainer::Matroska:
            return extension == "mkv";

        case VideoContainer::Mp4:
            return extension == "mp4";

        case VideoContainer::WebM:
            return extension == "webm";

        case VideoContainer::Mov:
            return extension == "mov";

        case VideoContainer::Avi:
            return extension == "avi";
    }

    return true;
}

bool FFmpegVideoExporter::initializeContainer(const VideoExportSettings& settings)
{
    const char* format = ffmpeg_utils::containerName(settings.container);

    const int ret = avformat_alloc_output_context2(&context_->formatContext, nullptr, format,
                                                   settings.filename.toUtf8().constData());

    if (ret < 0 || context_->formatContext == nullptr)
    {
        qWarning() << "avformat_alloc_output_context2 failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::initializeCodec(const VideoExportSettings& settings)
{
    const auto codecInfo = FFmpegCodecUtils::codecInfo(settings.codec);

    if (!codecInfo)
        return false;

    context_->codec = codecInfo->encoder;

    context_->codecContext = avcodec_alloc_context3(context_->codec);

    if (context_->codecContext == nullptr)
        return false;

    auto* c = context_->codecContext;

    c->codec_id = context_->codec->id;
    c->codec_type = AVMEDIA_TYPE_VIDEO;

    c->width = frameSize_.width();
    c->height = frameSize_.height();

    c->time_base = AVRational{1, settings.frameRate};
    c->framerate = AVRational{settings.frameRate, 1};

    c->pix_fmt = codecInfo->pixelFormat;

    if (context_->formatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    const int ret = avcodec_open2(c, context_->codec, nullptr);

    if (ret < 0)
    {
        qWarning() << "avcodec_open2 failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::initializeStream()
{
    context_->stream = avformat_new_stream(context_->formatContext, nullptr);

    if (context_->stream == nullptr)
        return false;

    context_->stream->time_base = context_->codecContext->time_base;

    const int ret =
        avcodec_parameters_from_context(context_->stream->codecpar, context_->codecContext);

    if (ret < 0)
    {
        qWarning() << "avcodec_parameters_from_context failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
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

    if (ret < 0)
    {
        qWarning() << "av_frame_get_buffer failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::allocatePacket()
{
    context_->packet = av_packet_alloc();

    return context_->packet != nullptr;
}

bool FFmpegVideoExporter::initializeScaler()
{
    //
    // BGR0 does not require any conversion.
    //
    // All other pixel formats are generated from the BGRA QImage
    // using libswscale.
    //

    if (context_->codecContext->pix_fmt == AV_PIX_FMT_BGR0)
        return true;

    auto* c = context_->codecContext;

    context_->swsContext = sws_getContext(c->width, c->height, AV_PIX_FMT_BGRA, c->width, c->height,
                                          c->pix_fmt, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    return context_->swsContext != nullptr;
}

bool FFmpegVideoExporter::openOutputFile()
{
    if (context_->formatContext == nullptr)
        return false;

    if (context_->formatContext->oformat->flags & AVFMT_NOFILE)
        return true;

    const QByteArray filename = settings_.filename.toUtf8();

    const int ret = avio_open(&context_->formatContext->pb, filename.constData(), AVIO_FLAG_WRITE);

    if (ret < 0)
    {
        qWarning() << "avio_open failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::writeHeader()
{
    const int ret = avformat_write_header(context_->formatContext, nullptr);

    if (ret < 0)
    {
        qWarning() << "avformat_write_header failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::makeFrameWritable()
{
    const int ret = av_frame_make_writable(context_->frame);

    if (ret < 0)
    {
        qWarning() << "av_frame_make_writable failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::fillFrame(const QImage& image)
{
    assert(state_ == ExportState::Recording);

    if (!makeFrameWritable())
        return false;

    //
    // Dispatch according to the encoder pixel format.
    //

    switch (context_->codecContext->pix_fmt)
    {
        case AV_PIX_FMT_BGR0:
            return fillFrameBgr0(image);

        case AV_PIX_FMT_YUV420P:
            return fillFrameYuv420(image);

        default:
            qWarning() << "Unsupported pixel format:"
                       << av_get_pix_fmt_name(context_->codecContext->pix_fmt);
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
    // QImage::Format_ARGB32 is stored as BGRA in memory on little-endian systems,
    // which is compatible with AV_PIX_FMT_BGR0.
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

    assert(src.width() == context_->frame->width);
    assert(src.height() == context_->frame->height);

    if (src.format() != QImage::Format_ARGB32)
    {
        src = src.convertToFormat(QImage::Format_ARGB32);
    }

    const uint8_t* srcData[1] = {src.constBits()};

    const int srcStride[1] = {static_cast<int>(src.bytesPerLine())};

    assert(context_->swsContext != nullptr);
    assert(context_->frame != nullptr);

    const int ret = sws_scale(context_->swsContext, srcData, srcStride, 0, src.height(),
                              context_->frame->data, context_->frame->linesize);

    if (ret != src.height())
    {
        qWarning() << "sws_scale failed.";

        return false;
    }

    return true;
}

bool FFmpegVideoExporter::encodeFrame()
{
    assert(state_ == ExportState::Recording);

    auto* frame = context_->frame;

    frame->pts = context_->nextPts++;

    const int ret = avcodec_send_frame(context_->codecContext, frame);

    if (ret < 0)
    {
        qWarning() << "avcodec_send_frame failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return receivePackets();
}

bool FFmpegVideoExporter::receivePackets()
{
    auto* packet = context_->packet;

    //
    // Retrieve all available encoded packets.
    //

    while (true)
    {
        const int receiveRet = avcodec_receive_packet(context_->codecContext, packet);

        if (receiveRet == AVERROR(EAGAIN) || receiveRet == AVERROR_EOF)
            break;

        if (receiveRet < 0)
        {
            qWarning() << "avcodec_receive_packet failed:" << ffmpeg_utils::errorString(receiveRet);

            return false;
        }

        av_packet_rescale_ts(packet, context_->codecContext->time_base,
                             context_->stream->time_base);

        packet->stream_index = context_->stream->index;

        const int writeRet = av_interleaved_write_frame(context_->formatContext, packet);

        av_packet_unref(packet);

        if (writeRet < 0)
        {
            qWarning() << "av_interleaved_write_frame failed:"
                       << ffmpeg_utils::errorString(writeRet);

            return false;
        }
    }

    return true;
}

bool FFmpegVideoExporter::flushEncoder()
{
    //
    // Signal end-of-stream to the encoder.
    //
    const int ret = avcodec_send_frame(context_->codecContext, nullptr);

    if (ret < 0)
    {
        qWarning() << "avcodec_send_frame failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    //
    // Drain all remaining packets.
    //
    return receivePackets();
}

bool FFmpegVideoExporter::writeTrailer()
{
    const int ret = av_write_trailer(context_->formatContext);

    if (ret < 0)
    {
        qWarning() << "av_write_trailer failed:" << ffmpeg_utils::errorString(ret);

        return false;
    }

    return true;
}

void FFmpegVideoExporter::release()
{
    //
    // Most FFmpeg deallocation helpers taking a pointer-to-pointer automatically
    // reset the pointer to nullptr.
    //

    if (context_->swsContext != nullptr)
    {
        sws_freeContext(context_->swsContext);
        context_->swsContext = nullptr;
    }

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

    frameSize_ = {};
}

} // namespace fluvel