// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_codec_utils.hpp"

extern "C"
{
#include <libavcodec/version.h>
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

/**
 * @brief Retrieves the pixel formats supported by an FFmpeg encoder.
 *
 * @param encoder FFmpeg encoder.
 *
 * @return Null-terminated list of supported pixel formats, or @c nullptr
 *         if no explicit pixel format restriction is reported.
 */
static const AVPixelFormat* encoderPixelFormats(const AVCodec* encoder)
{
    assert(encoder != nullptr);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(61, 13, 100)

    const AVPixelFormat* formats = nullptr;

    const int ret = avcodec_get_supported_config(nullptr, encoder, AV_CODEC_CONFIG_PIX_FORMAT, 0,
                                                 reinterpret_cast<const void**>(&formats), nullptr);

    if (ret < 0)
        return nullptr;

    return formats;

#else

    return encoder->pix_fmts;

#endif
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
        for (const char* encoderName : entry.preferredEncoders)
        {
            const AVCodec* encoder = avcodec_find_encoder_by_name(encoderName);

            if (encoder == nullptr)
                continue;

            const AVPixelFormat* pixelFormats = encoderPixelFormats(encoder);

            bool usable = false;

            if (pixelFormats != nullptr)
            {
                for (const AVPixelFormat* fmt = pixelFormats; *fmt != AV_PIX_FMT_NONE; ++fmt)
                {
                    if (isEncoderUsable(encoder, entry.codecId, *fmt))
                    {
                        usable = true;
                        break;
                    }
                }
            }
            else
            {
                //
                // FFmpeg does not report an explicit pixel-format restriction.
                // Validate the encoder with a reasonable default format.
                //
                usable = isEncoderUsable(encoder, entry.codecId, AV_PIX_FMT_YUV420P);
            }

            if (!usable)
                continue;

            CodecInfo info;

            info.codec = entry.codec;
            info.lossless = entry.lossless;
            info.encoder = encoder;

            codecs.push_back(std::move(info));

            //
            // The first usable encoder in the preference list wins.
            //
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

    const AVPixelFormat* pixelFormats = encoderPixelFormats(encoder);

    if (pixelFormats == nullptr)
        return true;

    for (const AVPixelFormat* fmt = pixelFormats; *fmt != AV_PIX_FMT_NONE; ++fmt)
    {
        if (*fmt == pixelFormat)
            return true;
    }

    return false;
}

} // namespace fluvel