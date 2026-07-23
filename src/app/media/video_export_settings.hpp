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

#include "recording_buffer_settings.hpp"
#include "recording_types.hpp"

#include <QSize>
#include <QString>

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
   *
   * The codecs are ordered by their typical usage rather than
   * by their release date.
   */
  enum class VideoCodec
  {
      /**
       * @brief FFV1 lossless codec.
       *
       * Best suited for scientific image processing and archival.
       * Provides mathematically lossless compression.
       */
      FFV1,

      /**
       * @brief H.264 / AVC codec.
       *
       * Excellent compatibility across operating systems,
       * web browsers and media players.
       */
      H264,

      /**
       * @brief H.265 / HEVC codec.
       *
       * Better compression efficiency than H.264 at the cost
       * of increased encoding complexity.
       */
      H265,

      /**
       * @brief AV1 codec.
       *
       * State-of-the-art open video codec providing excellent
       * compression efficiency. Encoding is computationally
       * demanding.
       */
      AV1,

      /**
       * @brief VP9 codec.
       *
       * Open codec mainly used with the WebM container.
       */
      VP9,

      /**
       * @brief MPEG-4 Part 2 Visual codec.
       *
       * Legacy codec historically used by DivX and Xvid.
       * Mostly kept for compatibility with older software
       * and hardware.
       */
      MPEG4Part2
  };

  /**
   * @brief Supported video containers.
   */
  enum class VideoContainer
  {
      /**
       * @brief Matroska container (.mkv).
       *
       * Recommended for lossless codecs such as FFV1.
       */
      Matroska,

      /**
       * @brief MPEG-4 container (.mp4).
       *
       * Widely supported by modern media players and devices.
       */
      Mp4,

      /**
       * @brief WebM container.
       *
       * Typically used with VP8 or VP9.
       */
      WebM,

      /**
       * @brief QuickTime container (.mov).
       */
      Mov,

      /**
       * @brief AVI container.
       *
       * Legacy multimedia container.
       */
      Avi
  };

  /**
   * @brief Video export settings.
   *
   * Defines the parameters used to export a recorded video, including
   * the output file, encoding options and buffering configuration.
   */
  struct VideoExportSettings
  {
      /**
       * @brief Output filename.
       *
       * The filename may be absolute or relative. If no extension is
       * provided, a suitable one may be automatically selected according
       * to the chosen export profile or container.
       */
      QString filename;

      /**
       * @brief Recording mode.
       *
       * Defines how the recording is stored.
       *
       * - @ref RecordingMode::SingleFile records everything into a single file.
       * - @ref RecordingMode::Circular records successive video segments and
       *   automatically removes the oldest ones according to the retention
       *   settings.
       */
      RecordingMode recordingMode{RecordingMode::SingleFile};

      /**
       * @brief Total retention time.
       *
       * In circular recording mode, the oldest recording segments are
       * automatically removed so that approximately this amount of video
       * history is preserved.
       *
       * Expressed in minutes.
       *
       * Ignored in single-file recording mode.
       */
      int retentionTimeMinutes{30};

      /**
       * @brief Number of recording segments.
       *
       * The retention time is divided into this number of segments.
       *
       * Larger values produce shorter segments, while smaller values
       * produce longer segments.
       *
       * Ignored in single-file recording mode.
       */
      int segmentCount{10};

      /**
       * @brief Target frame rate.
       *
       * Specifies the frame rate written to the exported video.
       * This value should normally match the acquisition frame rate.
       */
      int fps{30};

      /**
       * @brief Export profile.
       *
       * Fluvel is primarily a computer vision application; therefore,
       * Archive is the default profile. When set to Custom, the codec
       * and container specified below are used.
       */
      ExportProfile profile{ExportProfile::Archive};

      /**
       * @brief Video codec.
       *
       * Used only when @ref profile is set to @ref ExportProfile::Custom.
       */
      VideoCodec codec{VideoCodec::FFV1};

      /**
       * @brief Video container.
       *
       * Used only when @ref profile is set to @ref ExportProfile::Custom.
       */
      VideoContainer container{VideoContainer::Matroska};

      /**
       * @brief Recording buffer configuration.
       *
       * Defines the buffering strategy used during recording, including
       * RAM limits, temporary storage limits and overflow handling.
       */
      RecordingBufferSettings bufferSettings;
  };

} // namespace fluvel
