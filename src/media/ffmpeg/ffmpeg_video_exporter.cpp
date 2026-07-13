// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_video_exporter.hpp"

#include "ffmpeg_codec_utils.hpp"
#include "ffmpeg_utils.hpp"
#include "frame_pipeline.hpp"
#include "video_export_settings.hpp"
#include "video_exporter_utils.hpp"

#include <QDebug>
#include <QFileInfo>

#include <cassert>

extern "C"
{
#include <libavformat/avformat.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
}

namespace
{

//
// Preferred pixel formats.
//
// The formats are ordered to minimize conversions from the current
// input representation. The first compatible format is selected.
//
static constexpr AVPixelFormat kGray8Formats[] = {AV_PIX_FMT_GRAY8, AV_PIX_FMT_YUV420P,
                                                  AV_PIX_FMT_NONE};

static constexpr AVPixelFormat kBgraFormats[] = {AV_PIX_FMT_BGR0, AV_PIX_FMT_BGRA,
                                                 AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};

/**
 * @brief Returns the preferred FFmpeg pixel formats for a QImage format.
 *
 * The returned null-terminated list is ordered by Fluvel preference.
 *
 * @param format Source QImage pixel format.
 * @return Preferred FFmpeg pixel formats, or @c nullptr if the format is unsupported.
 */
const AVPixelFormat* preferredPixelFormats(QImage::Format format)
{
    switch (format)
    {
        case QImage::Format_Grayscale8:
            return kGray8Formats;

        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            return kBgraFormats;

        default:
            return nullptr;
    }
}

/**
 * @brief Selects the best encoder pixel format for the input image.
 *
 * The preferred pixel formats are evaluated in order until one supported
 * by the encoder is found.
 *
 * @param encoder FFmpeg encoder.
 * @param preferredPixelFormats Null-terminated list of preferred pixel formats.
 * @return Selected pixel format, or @c AV_PIX_FMT_NONE if none is compatible.
 */
AVPixelFormat selectPixelFormat(const AVCodec* encoder, const AVPixelFormat* preferredPixelFormats)
{
    assert(encoder != nullptr);
    assert(preferredPixelFormats != nullptr);

    for (const AVPixelFormat* fmt = preferredPixelFormats; *fmt != AV_PIX_FMT_NONE; ++fmt)
    {
        if (fluvel::FFmpegCodecUtils::supportsPixelFormat(encoder, *fmt))
            return *fmt;
    }

    return AV_PIX_FMT_NONE;
}

} // namespace

namespace fluvel
{

/**
 * @brief Internal FFmpeg exporter context.
 *
 * Stores the FFmpeg objects and runtime state required during a video
 * export session.
 */
struct FFmpegVideoExporter::Context
{
    //
    // FFmpeg objects
    //

    /// Output format context.
    AVFormatContext* formatContext{nullptr};

    /// Selected video encoder.
    const AVCodec* codec{nullptr};

    /// Video encoder context.
    AVCodecContext* codecContext{nullptr};

    /// Output video stream.
    AVStream* stream{nullptr};

    /// Video frame submitted to the encoder.
    AVFrame* frame{nullptr};

    /// Packet containing encoded data.
    AVPacket* packet{nullptr};

    /// Pixel format conversion context.
    SwsContext* swsContext{nullptr};

    //
    // Encoding state
    //

    /// Index of the next frame in constant frame rate mode.
    int64_t frameIndex{0};

    /// Timestamp of the first frame in explicit timestamp mode (ns).
    int64_t firstTimestampNs{-1};
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

    if (!exporter_utils::hasExpectedExtension(settings_.filename, settings_.container))
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

bool FFmpegVideoExporter::addFrame(const VideoFrame& frame)
{
    QImage image = frame.image;

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

    if (image.format() != frameFormat_)
    {
        qWarning() << "Frame format" << image.format() << "does not match initial frame format"
                   << frameFormat_;

        return false;
    }

    if (!fillFrame(image) || !updateFrameTimestamp(frame) || !encodeFrame())
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
    frameFormat_ = firstFrame.format();

    const AVPixelFormat* preferredFormats = preferredPixelFormats(frameFormat_);

    if (preferredFormats == nullptr)
    {
        qWarning() << "Unsupported QImage format:" << frameFormat_;
        return false;
    }

    if (!initializeContainer(settings_) || !initializeCodec(settings_, preferredFormats) ||
        !initializeStream() || !allocateFrame() || !allocatePacket() || !initializeScaler() ||
        !openOutputFile() || !writeHeader())
    {
        release();
        state_ = ExportState::Closed;
        return false;
    }

    state_ = ExportState::Recording;

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

bool FFmpegVideoExporter::initializeCodec(const VideoExportSettings& settings,
                                          const AVPixelFormat* preferredPixelFormats)
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

    c->time_base = AVRational{1, settings.fps};
    c->framerate = AVRational{settings.fps, 1};

    c->pix_fmt = selectPixelFormat(context_->codec, preferredPixelFormats);

    if (c->pix_fmt == AV_PIX_FMT_NONE)
    {
        qWarning() << "No compatible pixel format found for encoder" << context_->codec->name;

        return false;
    }

    if (context_->formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

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
    //
    // All other supported pixel formats are generated from the
    // BGRA input image using libswscale.
    //
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

    if (image.format() != QImage::Format_RGB32)
    {
        converted = image.convertToFormat(QImage::Format_RGB32);
        src = &converted;
    }

    auto* frame = context_->frame;

    constexpr int kBytesPerPixel = 4;
    const int bytesPerRow = src->width() * kBytesPerPixel;

    //
    // QImage::Format_RGB32 is stored as BGRX in memory on little-endian systems,
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

    if (src.format() != QImage::Format_RGB32)
    {
        src = src.convertToFormat(QImage::Format_RGB32);
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

bool FFmpegVideoExporter::updateFrameTimestamp(const VideoFrame& frame)
{
    // Use constant frame rate timing when no explicit timestamp is provided.
    if (!frame.presentationTimestampNs)
    {
        context_->frame->pts = context_->frameIndex++;
        return true;
    }

    // Normalize explicit timestamps relative to the first frame.
    if (context_->firstTimestampNs < 0)
        context_->firstTimestampNs = *frame.presentationTimestampNs;

    const int64_t timestampNs = *frame.presentationTimestampNs - context_->firstTimestampNs;

    context_->frame->pts =
        av_rescale_q(timestampNs, AVRational{1, 1'000'000'000}, context_->codecContext->time_base);

    return true;
}

bool FFmpegVideoExporter::encodeFrame()
{
    assert(state_ == ExportState::Recording);

    const int ret = avcodec_send_frame(context_->codecContext, context_->frame);

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
    // Runtime export state
    //
    context_->codec = nullptr;
    context_->stream = nullptr;

    context_->frameIndex = 0;
    context_->firstTimestampNs = -1;

    frameSize_ = {-1, -1};
    frameFormat_ = QImage::Format_Invalid;
}

} // namespace fluvel