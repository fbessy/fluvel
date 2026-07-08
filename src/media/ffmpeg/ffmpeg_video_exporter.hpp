// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"
#include "video_exporter_backend.hpp"

extern "C"
{
// #include <libavutil/pixfmt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
}

#include <memory>

class QImage;

namespace fluvel
{

/**
 * @brief FFmpeg implementation of the video exporter.
 *
 * This class implements the IVideoExporter interface using the
 * FFmpeg multimedia framework.
 *
 * The public API remains backend-independent.
 */
class FFmpegVideoExporter final : public IVideoExporter
{
public:
    /**
     * @brief Constructs a FFmpeg video exporter.
     */
    FFmpegVideoExporter();

    /**
     * @brief Destroys the exporter.
     */
    ~FFmpegVideoExporter() override;

    FFmpegVideoExporter(const FFmpegVideoExporter&) = delete;
    FFmpegVideoExporter& operator=(const FFmpegVideoExporter&) = delete;

    FFmpegVideoExporter(FFmpegVideoExporter&&) = delete;
    FFmpegVideoExporter& operator=(FFmpegVideoExporter&&) = delete;

    /**
     * @copydoc IVideoExporter::open
     */
    [[nodiscard]]
    bool open(const VideoExportSettings& settings) override;

    /**
     * @copydoc IVideoExporter::addFrame
     */
    [[nodiscard]]
    bool addFrame(const QImage& image) override;

    // bool addFrame(const QVideoFrame& frame);

    /**
     * @copydoc IVideoExporter::close
     */
    [[nodiscard]]
    bool close() override;

    /**
     * @copydoc IVideoExporter::isOpen
     */
    [[nodiscard]]
    bool isOpen() const override;

private:
    /**
     * @brief Applies an export profile.
     *
     * Converts high-level export profiles into explicit codec
     * and container selections.
     *
     * @param settings Export settings to update.
     */
    void applyExportProfile(VideoExportSettings& settings) const;

    /**
     * @brief Initializes the output container.
     *
     * @param settings Export settings.
     * @return @c true on success, @c false otherwise.
     */
    bool initializeContainer(const VideoExportSettings& settings);

    /**
     * @brief Initializes the video codec.
     *
     * @param settings Export settings.
     * @return @c true on success, @c false otherwise.
     */
    bool initializeCodec(const VideoExportSettings& settings);

    /**
     * @brief Initializes the output stream.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool initializeStream();

    /**
     * @brief Allocates the video frame.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool allocateFrame();

    /**
     * @brief Allocates the packet used for encoded data.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool allocatePacket();

    /**
     * @brief Initializes the pixel format conversion context.
     *
     * Creates the libswscale context when a conversion from the input image
     * format to the encoder pixel format is required.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool initializeScaler();

    /**
     * @brief Opens the output file.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool openOutputFile();

    /**
     * @brief Writes the container header.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool writeHeader();

    /**
     * @brief Makes the frame writable.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool makeFrameWritable();

    /**
     * @brief Fills the encoder frame from an image.
     *
     * Dispatches the operation according to the encoder pixel format.
     *
     * @param image Source image.
     * @return @c true on success, @c false otherwise.
     */
    bool fillFrame(const QImage& image);

    /**
     * @brief Copies a BGRA image into the encoder frame.
     *
     * @param image Source image.
     * @return @c true on success, @c false otherwise.
     */
    bool fillFrameBgr0(const QImage& image);

    /**
     * @brief Converts an image to YUV420 and fills the encoder frame.
     *
     * @param image Source image.
     * @return @c true on success, @c false otherwise.
     */
    bool fillFrameYuv420(const QImage& image);

    /**
     * @brief Encodes the current frame.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool encodeFrame();

    /**
     * @brief Retrieves and writes all available encoded packets.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool receivePackets();

    /**
     * @brief Flushes the encoder.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool flushEncoder();

    /**
     * @brief Writes the container trailer.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool writeTrailer();

    /**
     * @brief Releases all FFmpeg resources.
     */
    void release();

    VideoExportSettings settings_;

    bool isOpen_{false};

    //
    // FFmpeg objects
    //
    // They are intentionally hidden from the public header.
    // Concrete FFmpeg types will appear only in the .cpp.
    //

    struct Context;
    std::unique_ptr<Context> context_;
};

} // namespace fluvel