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

    /**
     * @brief Checks whether an encoder supports a pixel format.
     *
     * @param encoder FFmpeg video encoder.
     * @param pixelFormat Pixel format to test.
     * @return True if the encoder supports the pixel format, false otherwise.
     */
    static bool supportsPixelFormat(const AVCodec* encoder, AVPixelFormat pixelFormat);

private:
    /**
     * @brief Detects all video codecs available on the current system.
     *
     * For each codec, the detection tries the preferred FFmpeg encoders
     * in priority order. For each encoder, it tests the supported pixel
     * formats until a compatible configuration can be successfully
     * initialized.
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
};

} // namespace fluvel