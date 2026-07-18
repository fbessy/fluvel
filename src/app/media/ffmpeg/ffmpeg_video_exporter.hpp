// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"
#include "video_exporter_backend.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
}

#include <memory>

#include <QImage>

namespace fluvel
{

/**
 * @brief Internal state of the exporter.
 *
 * The exporter is initially closed. Calling open() starts a new export
 * session but defers the FFmpeg initialization until the first frame is
 * received. Once initialized, frames can be encoded until close() is
 * called.
 */
enum class ExportState
{
    /// No export session is active.
    Closed,

    /// Waiting for the first frame to determine the output format
    /// and to initialize FFmpeg.
    WaitingForFirstFrame,

    /// Export session initialized and ready to encode frames.
    Recording
};

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
    bool addFrame(const VideoFrame& frame) override;

    /**
     * @copydoc IVideoExporter::close
     */
    [[nodiscard]]
    bool close() override;

    /**
     * @copydoc IVideoExporter::isRecording
     */
    [[nodiscard]]
    bool isRecording() const override;

    /**
     * @brief Returns the video codecs available through the FFmpeg backend.
     *
     * @return Available video codecs.
     */
    QList<VideoCodec> availableCodecs() const override;

private:
    /**
     * @brief Initializes the exporter from the first input frame.
     *
     * The first frame defines the output image characteristics and
     * triggers the FFmpeg initialization (container, codec, stream,
     * frame allocation, scaler, output file and header).
     *
     * @param firstFrame First frame to export.
     * @return @c true on success, @c false otherwise.
     */
    bool initializeFromFirstFrame(const QImage& firstFrame);

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
     * @param preferredFormats Preferred pixel formats ordered by priority.
     * @return @c true on success, @c false otherwise.
     */
    bool initializeCodec(const VideoExportSettings& settings,
                         const AVPixelFormat* preferredFormats);

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
     * @brief Fills the encoder frame with grayscale image data.
     *
     * The source image is converted to @c QImage::Format_Grayscale8 when required
     * and copied into the @c AV_PIX_FMT_GRAY8 frame buffer.
     *
     * @param image Source image.
     * @return @c true on success, otherwise @c false.
     */
    bool fillFrameGray8(const QImage& image);

    /**
     * @brief Fills the encoder frame with RGB24 image data.
     *
     * The source image is converted to @c QImage::Format_RGB888 when required
     * and copied into the @c AV_PIX_FMT_RGB24 frame buffer.
     *
     * @param image Source image.
     * @return @c true on success, otherwise @c false.
     */
    bool fillFrameRgb24(const QImage& image);

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
     * @brief Updates the presentation timestamp of the encoder frame.
     *
     * The timestamp is generated from the configured frame rate or derived
     * from the timestamp provided by the input frame, depending on the
     * configured timestamp mode.
     *
     * @param frame Input video frame.
     * @return @c true on success, @c false otherwise.
     */
    bool updateFrameTimestamp(const VideoFrame& frame);

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
     * @brief Releases all allocated FFmpeg resources.
     *
     * Restores the exporter to an uninitialized state.
     */
    void release();

    VideoExportSettings settings_;

    ExportState state_{ExportState::Closed};

    QSize frameSize_{-1, -1};
    QImage::Format frameFormat_{QImage::Format_Invalid};
    int64_t lastPts_{AV_NOPTS_VALUE}; // last presentation timestamp

    struct Context;
    std::unique_ptr<Context> context_;
};

} // namespace fluvel