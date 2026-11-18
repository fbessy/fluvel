// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_codec_utils.hpp"

extern "C"
{
#include <libavcodec/version.h>
#include <libavutil/error.h>
#include <libavutil/pixdesc.h>
}

#include <QStringList>

#include <array>
#include <cassert>
#include <cerrno>
#include <iterator>
#include <span>

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
    std::span<const char* const> preferredEncoders;

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
// Each list lives in its own static array so that the spans below never
// point to temporary storage (std::initializer_list backing arrays inside a
// constexpr table are not reliable with MSVC).
static constexpr std::array kFfv1Encoders = {"ffv1"};
static constexpr std::array kMpeg4Encoders = {"mpeg4"};
static constexpr std::array kH264Encoders = {
    "h264_nvenc",        // NVIDIA
    "h264_amf",          // AMD
    "h264_qsv",          // Intel
    "h264_videotoolbox", // Apple
    "h264_vaapi",        // Generic Linux
    "libx264"            // Software
};
static constexpr std::array kH265Encoders = {"hevc_nvenc", "hevc_amf",  "hevc_qsv",
                                             "hevc_videotoolbox", "hevc_vaapi", "libx265"};
static constexpr std::array kVp9Encoders = {"vp9_vaapi", "libvpx-vp9"};
static constexpr std::array kAv1Encoders = {"av1_nvenc", "av1_amf",   "av1_qsv",
                                            "av1_videotoolbox", "av1_vaapi", "libsvtav1",
                                            "librav1e", "libaom-av1"};

static constexpr CodecEntry kCodecTable[] = {
    {VideoCodec::FFV1, AV_CODEC_ID_FFV1, kFfv1Encoders, true},
    {VideoCodec::MPEG4Part2, AV_CODEC_ID_MPEG4, kMpeg4Encoders, false},
    {VideoCodec::H264, AV_CODEC_ID_H264, kH264Encoders, false},
    {VideoCodec::H265, AV_CODEC_ID_HEVC, kH265Encoders, false},
    {VideoCodec::VP9, AV_CODEC_ID_VP9, kVp9Encoders, false},
    {VideoCodec::AV1, AV_CODEC_ID_AV1, kAv1Encoders, false},
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

/**
 * @brief Tries to open an encoder with a small dummy configuration.
 *
 * @return 0 on success, a negative FFmpeg error code otherwise.
 */
static int tryOpenEncoder(const AVCodec* encoder, AVCodecID codecId, AVPixelFormat pixelFormat)
{
    AVCodecContext* ctx = avcodec_alloc_context3(encoder);

    if (ctx == nullptr)
        return AVERROR(ENOMEM);

    ctx->codec_id = codecId;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->width = 16;
    ctx->height = 16;
    ctx->time_base = AVRational{1, 25};
    ctx->framerate = AVRational{25, 1};
    ctx->pix_fmt = pixelFormat;

    const int ret = avcodec_open2(ctx, encoder, nullptr);

    avcodec_free_context(&ctx);

    return ret < 0 ? ret : 0;
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

QString FFmpegCodecUtils::diagnostics()
{
    QStringList lines;

    lines << QStringLiteral("avcodec %1.%2.%3")
                 .arg(LIBAVCODEC_VERSION_MAJOR)
                 .arg(LIBAVCODEC_VERSION_MINOR)
                 .arg(LIBAVCODEC_VERSION_MICRO);
    lines << QStringLiteral("codec table entries: %1").arg(std::size(kCodecTable));

    for (const CodecEntry& entry : kCodecTable)
    {
        lines << QStringLiteral("codec id %1: %2 preferred encoder(s)")
                     .arg(static_cast<int>(entry.codecId))
                     .arg(entry.preferredEncoders.size());

        for (const char* encoderName : entry.preferredEncoders)
        {
            const AVCodec* encoder = avcodec_find_encoder_by_name(encoderName);

            if (encoder == nullptr)
            {
                lines << QStringLiteral("  %1: not found").arg(QString::fromLatin1(encoderName));
                continue;
            }

            const AVPixelFormat* formats = encoderPixelFormats(encoder);
            const AVPixelFormat fmt = formats ? formats[0] : AV_PIX_FMT_YUV420P;

            const int ret = tryOpenEncoder(encoder, entry.codecId, fmt);

            if (ret == 0)
            {
                lines << QStringLiteral("  %1: ok").arg(QString::fromLatin1(encoderName));
            }
            else
            {
                char err[AV_ERROR_MAX_STRING_SIZE] = {};
                av_strerror(ret, err, sizeof(err));

                lines << QStringLiteral("  %1: open failed (%2, pix_fmt %3)")
                             .arg(QString::fromLatin1(encoderName), QString::fromLatin1(err))
                             .arg(static_cast<int>(fmt));
            }
        }
    }

    return lines.join(QLatin1Char('\n'));
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