// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"

#include <QList>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include <optional>

namespace fluvel
{

/**
 * @brief Describes an available FFmpeg video codec.
 *
 * A codec is considered available when:
 * - a suitable FFmpeg encoder is found;
 * - a compatible pixel format is selected;
 * - the encoder can be successfully opened.
 */
struct CodecInfo
{
    /// Fluvel video codec.
    VideoCodec codec;

    /// Indicates whether the codec uses lossless compression.
    bool lossless;

    /// Selected FFmpeg encoder.
    const AVCodec* encoder;

    /// Preferred pixel format for the encoder.
    AVPixelFormat pixelFormat;
};

class FFmpegCodecUtils
{
public:
    /**
     * @brief Returns all video codecs available on the current system.
     *
     * The returned list is cached after the first call.
     *
     * @return Available codec descriptions.
     */
    static const QList<CodecInfo>& availableCodecs();

    /**
     * @brief Returns information about a specific codec.
     *
     * @param codec Fluvel video codec.
     * @return Codec information if available, std::nullopt otherwise.
     */
    static std::optional<CodecInfo> codecInfo(VideoCodec codec);

private:
    /**
     * @brief Detects all video codecs available on the current system.
     *
     * The detection verifies that a suitable encoder exists, selects a
     * compatible pixel format and ensures that the encoder can be opened.
     *
     * @return List of available codec descriptions.
     */
    static QList<CodecInfo> detectAvailableCodecs();

    /**
     * @brief Checks whether an encoder can be successfully opened.
     *
     * @param encoder FFmpeg encoder.
     * @param codecId FFmpeg codec identifier.
     * @param pixelFormat Pixel format used for the validation.
     * @return @c true if the encoder can be opened, @c false otherwise.
     */
    static bool isEncoderUsable(const AVCodec* encoder, AVCodecID codecId,
                                AVPixelFormat pixelFormat);

    /**
     * @brief Returns the preferred encoder for a video codec.
     *
     * The preferred software encoder is used when available. Otherwise,
     * FFmpeg's default encoder for the codec is returned.
     *
     * @param codec Fluvel video codec.
     * @return Selected FFmpeg encoder, or @c nullptr if none is available.
     */
    static const AVCodec* findEncoder(VideoCodec codec);

    /**
     * @brief Selects the preferred pixel format for an encoder.
     *
     * @param encoder FFmpeg encoder.
     * @return Preferred pixel format, or @c AV_PIX_FMT_NONE if no suitable
     *         format is supported.
     */
    static AVPixelFormat preferredPixelFormat(const AVCodec* encoder);
};

} // namespace fluvel