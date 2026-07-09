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
 * @brief Associates a Fluvel video codec with its preferred FFmpeg encoders.
 *
 * The encoders are ordered by preference. Fluvel always selects the first
 * encoder that is available and can be successfully initialized on the
 * current system.
 *
 * Hardware-accelerated encoders are preferred over software implementations
 * whenever possible.
 */
struct CodecEntry
{
    /// Fluvel video codec.
    VideoCodec codec;

    /// FFmpeg codec identifier.
    AVCodecID codecId;

    /// Preferred FFmpeg encoders ordered by priority.
    std::initializer_list<const char*> preferredEncoders;

    /// Indicates whether the codec uses lossless compression.
    bool lossless;
};

//
// Preferred FFmpeg encoders.
//
// Encoders are ordered by Fluvel preference.
//
// Vendor-specific hardware encoders are preferred over generic hardware
// APIs and software implementations. The order between vendor-specific
// encoders provides deterministic selection only; in practice, most of
// them are mutually exclusive depending on the operating system and the
// available hardware.
//
static constexpr CodecEntry kCodecTable[] = {
    {VideoCodec::FFV1, AV_CODEC_ID_FFV1, {"ffv1"}, true},
    {VideoCodec::MPEG4Part2, AV_CODEC_ID_MPEG4, {"mpeg4"}, false},
    {VideoCodec::H264,
     AV_CODEC_ID_H264,
     {
         "h264_nvenc",        // NVIDIA
         "h264_amf",          // AMD
         "h264_qsv",          // Intel
         "h264_videotoolbox", // Apple
         "h264_vaapi",        // Generic Linux
         "libx264"            // Software
     },
     false},
    {VideoCodec::H265,
     AV_CODEC_ID_HEVC,
     {"hevc_nvenc", "hevc_amf", "hevc_qsv", "hevc_videotoolbox", "hevc_vaapi", "libx265"},
     false},
    {VideoCodec::VP9, AV_CODEC_ID_VP9, {"vp9_vaapi", "libvpx-vp9"}, false},
    {VideoCodec::AV1,
     AV_CODEC_ID_AV1,
     {"av1_nvenc", "av1_amf", "av1_qsv", "av1_videotoolbox", "av1_vaapi", "libsvtav1", "librav1e",
      "libaom-av1"},
     false},
};

//
// Preferred pixel formats.
//
// The formats are ordered to minimize conversions from the current
// input representation. The first compatible format is selected.
//
static constexpr AVPixelFormat kPreferredPixelFormats[] = {AV_PIX_FMT_BGR0, AV_PIX_FMT_BGRA,
                                                           AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};

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
        bool found = false;

        for (const char* encoderName : entry.preferredEncoders)
        {
            const AVCodec* encoder = avcodec_find_encoder_by_name(encoderName);

            if (encoder == nullptr)
                continue;

            for (AVPixelFormat pixelFormat : kPreferredPixelFormats)
            {
                if (pixelFormat == AV_PIX_FMT_NONE)
                    break;

                if (!supportsPixelFormat(encoder, pixelFormat))
                    continue;

                if (!isEncoderUsable(encoder, entry.codecId, pixelFormat))
                    continue;

                CodecInfo info;

                info.codec = entry.codec;
                info.lossless = entry.lossless;
                info.encoder = encoder;
                info.pixelFormat = pixelFormat;

                codecs.push_back(std::move(info));

                found = true;
                break;
            }

            if (found)
                break;
        }
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

bool FFmpegCodecUtils::supportsPixelFormat(const AVCodec* encoder, AVPixelFormat pixelFormat)
{
    assert(encoder != nullptr);

    if (encoder->pix_fmts == nullptr)
        return false;

    for (const AVPixelFormat* supported = encoder->pix_fmts; *supported != AV_PIX_FMT_NONE;
         ++supported)
    {
        if (*supported == pixelFormat)
            return true;
    }

    return false;
}

} // namespace fluvel