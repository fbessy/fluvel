// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file
 *
 * This file defines the public video export API.
 *
 * The API is backend-independent.
 * Codec and container selection are implementation details.
 */

#pragma once

#include <QSize>
#include <QString>

#include <cstdint>

namespace fluvel
{

  /**
   * @brief Video export profiles.
   *
   * A profile represents the intended use of the exported video.
   * The backend automatically selects the most appropriate codec
   * and container for each profile.
   */
  enum class ExportProfile
  {
    /**
     * @brief Lossless archival.
     *
     * Intended for computer vision, scientific work,
     * debugging and long-term storage.
     *
     * Default backend:
     * - Codec: FFV1
     * - Container: Matroska (.mkv)
     */
    Archive,

    /**
     * @brief Maximum compatibility.
     *
     * Intended for video players, web browsers
     * and video sharing.
     *
     * Default backend:
     * - Codec: H.264
     * - Container: MP4
     */
    Compatible,

    /**
     * @brief Balanced quality, size and compatibility.
     *
     * Intended for everyday use.
     *
     * Default backend:
     * - Codec: H.265
     * - Container: MP4
     */
    Balanced,

    /**
     * @brief Best compression efficiency.
     *
     * Intended for minimizing file size while
     * preserving visual quality.
     *
     * Default backend:
     * - Codec: AV1
     * - Container: MP4
     */
    Efficient,

    /**
     * @brief User-defined codec and container.
     */
    Custom
  };

  /**
   * @brief Supported video codecs.
   */
  enum class VideoCodec
  {
    FFV1,
    H264,
    H265,
    MPEG4,
    VP9,
    AV1
  };

  /**
   * @brief Supported video containers.
   */
  enum class VideoContainer
  {
    Matroska,
    Mp4,
    Avi,
    Mov,
    WebM
  };

  /**
   * @brief Encoding speed / compression trade-off.
   */
  enum class VideoPreset
  {
      UltraFast,
      Fast,
      Medium,
      Slow,
      VerySlow
  };

  /**
   * @brief Video export settings.
   */
  struct VideoExportSettings
  {
    /**
     * @brief Output filename.
     */
    QString filename;

    /**
     * @brief Frame size.
     */
    QSize frameSize;

    /**
     * @brief Frames per second.
     */
    std::uint32_t fps{30};

    /**
     * @brief Export profile.
     *
     * Fluvel is primarily a computer vision application,
     * therefore Archive is the default profile.
     */
    ExportProfile profile{ExportProfile::Archive};

    /**
     * @brief Video codec.
     *
     * Used only when profile is Custom.
     */
    VideoCodec codec{VideoCodec::FFV1};

    /**
     * @brief Video container.
     *
     * Used only when profile is Custom.
     */
    VideoContainer container{VideoContainer::Matroska};

    /**
     * @brief Encoding preset.
     */
    VideoPreset preset{VideoPreset::Medium};

    /**
     * @brief Target bitrate.
     *
     * Used by lossy codecs.
     */
    std::uint32_t bitrate{8'000'000};

    /**
     * @brief Compression quality.
     *
     * Lower values generally produce higher quality.
     * Interpretation depends on the selected codec.
     */
    std::uint32_t quality{23};
  };

} // namespace fluvel
