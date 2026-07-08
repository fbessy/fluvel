// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"
#include "video_exporter_backend.hpp"

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
     *
     * @return True on success.
     */
    bool initializeContainer(const VideoExportSettings& settings);

    /**
     * @brief Initializes the video codec.
     *
     * @param settings Export settings.
     *
     * @return True on success.
     */
    bool initializeCodec(const VideoExportSettings& settings);

    /**
     * @brief Initializes the video stream.
     *
     * @return True on success.
     */
    bool initializeStream();

    /**
     * @brief Opens the output file.
     *
     * @return True on success.
     */
    bool openOutputFile();

    /**
     * @brief Writes the container header.
     *
     * @return True on success.
     */
    bool writeHeader();

    /**
     * @brief Flushes the encoder.
     *
     * @return True on success.
     */
    bool flushEncoder();

    /**
     * @brief Writes the container trailer.
     *
     * @return True on success.
     */
    bool writeTrailer();

    /**
     * @brief Releases all FFmpeg resources.
     */
    void release();

    bool allocateFrame();
    bool allocatePacket();
    bool initializeScaler();
    bool makeFrameWritable();

    bool receivePackets();

    bool fillFrame(const QImage& image);
    bool fillFrameBgr0(const QImage& image);
    bool fillFrameYuv420(const QImage& image);

    bool encodeFrame();

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