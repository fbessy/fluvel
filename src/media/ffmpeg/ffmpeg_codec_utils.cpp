// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_codec_utils.hpp"

extern "C"
{
#include <libavutil/pixdesc.h>
}

#include <cassert>

namespace fluvel
{

namespace
{

/**
 * @brief Codec mapping used by FFmpegCodecUtils.
 *
 * Each entry associates a Fluvel video codec with the corresponding FFmpeg
 * codec identifier and the preferred encoder name.
 */
struct CodecEntry
{
    /// Fluvel video codec.
    VideoCodec codec;

    /// FFmpeg codec identifier.
    AVCodecID codecId;

    /// Preferred FFmpeg software encoder.
    const char* preferredEncoder;
};

static constexpr CodecEntry kCodecTable[] = {
    {VideoCodec::FFV1, AV_CODEC_ID_FFV1, "ffv1"},
    {VideoCodec::MPEG4Part2, AV_CODEC_ID_MPEG4, "mpeg4"},
    {VideoCodec::H264, AV_CODEC_ID_H264, "libx264"},
    {VideoCodec::H265, AV_CODEC_ID_HEVC, "libx265"},
    {VideoCodec::VP9, AV_CODEC_ID_VP9, "libvpx-vp9"},
    {VideoCodec::AV1, AV_CODEC_ID_AV1, "libsvtav1"},
};

const CodecEntry* findCodecEntry(VideoCodec codec)
{
    for (const CodecEntry& entry : kCodecTable)
    {
        if (entry.codec == codec)
            return &entry;
    }

    return nullptr;
}

} // namespace

const QList<CodecInfo>& FFmpegCodecUtils::availableCodecs()
{
    static const QList<CodecInfo> codecs = detectAvailableCodecs();

    return codecs;
}

std::optional<CodecInfo> FFmpegCodecUtils::codecInfo(VideoCodec codec)
{
    const auto& codecs = availableCodecs();

    auto it = std::find_if(codecs.begin(), codecs.end(),
                           [codec](const CodecInfo& info)
                           {
                               return info.codec == codec;
                           });

    if (it == codecs.end())
        return std::nullopt;

    return *it;
}

QList<CodecInfo> FFmpegCodecUtils::detectAvailableCodecs()
{
    QList<CodecInfo> codecs;

    for (const CodecEntry& entry : kCodecTable)
    {
        const AVCodec* encoder = findEncoder(entry.codec);

        if (encoder == nullptr)
            continue;

        const AVPixelFormat pixelFormat = selectPixelFormat(encoder);

        if (pixelFormat == AV_PIX_FMT_NONE)
            continue;

        if (!isEncoderUsable(encoder, entry.codecId, pixelFormat))
            continue;

        CodecInfo info;

        info.codec = entry.codec;
        info.encoder = encoder;
        info.pixelFormat = pixelFormat;
        info.lossless = (entry.codec == VideoCodec::FFV1);

        codecs.push_back(std::move(info));
    }

    return codecs;
}

bool FFmpegCodecUtils::isEncoderUsable(const AVCodec* encoder, AVCodecID codecId,
                                       AVPixelFormat pixelFormat)
{
    assert(encoder != nullptr);

    AVCodecContext* ctx = avcodec_alloc_context3(encoder);

    if (ctx == nullptr)
        return false;

    ctx->codec_id = codecId;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;

    //
    // Small dummy configuration used only to validate the encoder.
    //
    ctx->width = 16;
    ctx->height = 16;

    ctx->time_base = AVRational{1, 25};
    ctx->framerate = AVRational{25, 1};

    ctx->pix_fmt = pixelFormat;

    const int ret = avcodec_open2(ctx, encoder, nullptr);

    avcodec_free_context(&ctx);

    return ret >= 0;
}

const AVCodec* FFmpegCodecUtils::findEncoder(VideoCodec codec)
{
    const CodecEntry* entry = findCodecEntry(codec);

    if (entry == nullptr)
        return nullptr;

    //
    // Prefer the configured encoder. If it is unavailable, fall back to
    // FFmpeg's default encoder for the requested codec.
    //
    if (const AVCodec* encoder = avcodec_find_encoder_by_name(entry->preferredEncoder))
    {
        return encoder;
    }

    //
    // Fallback to FFmpeg default encoder.
    //
    return avcodec_find_encoder(entry->codecId);
}

AVPixelFormat FFmpegCodecUtils::selectPixelFormat(const AVCodec* encoder)
{
    if (encoder == nullptr || encoder->pix_fmts == nullptr)
    {
        //
        // Encoder does not advertise supported pixel formats.
        //
        return AV_PIX_FMT_NONE;
    }

    //
    // Search formats in order of preference.
    //
    // The current input source is a QImage, whose memory layout is
    // closest to BGRA/BGR0. Those formats are therefore preferred over
    // formats requiring a color space conversion.
    //
    for (const AVPixelFormat* pixFmt = encoder->pix_fmts; *pixFmt != AV_PIX_FMT_NONE; ++pixFmt)
    {
        switch (*pixFmt)
        {
            case AV_PIX_FMT_BGR0:
                return *pixFmt;

            case AV_PIX_FMT_BGRA:
                return *pixFmt;

            case AV_PIX_FMT_YUV420P:
                //
                // Fallback requiring a BGRA -> YUV conversion.
                //
                return *pixFmt;

            default:
                break;
        }
    }
    //
    // TODO:
    // When exporting QVideoFrame objects, extend the selection policy to
    // choose the pixel format that is closest to the input frame format
    // rather than assuming a BGRA source.
    //

    //
    // No suitable format found.
    //
    return AV_PIX_FMT_NONE;
}

} // namespace fluvel