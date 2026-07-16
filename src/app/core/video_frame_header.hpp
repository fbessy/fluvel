// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QtTypes>
#include <type_traits>

namespace fluvel
{

/**
 * @brief Header describing a video frame stored in a spool file.
 *
 * This structure precedes the raw image data in the spool file and
 * contains all the information required to reconstruct a
 * VideoFrame.
 *
 * The pixel data is stored immediately after this header as an
 * uncompressed byte array whose size is given by @ref imageSize.
 */
struct VideoFrameHeader
{
    /**
     * @brief Image width in pixels.
     */
    quint32 width{0};

    /**
     * @brief Image height in pixels.
     */
    quint32 height{0};

    /**
     * @brief QImage pixel format.
     *
     * Stores the numeric value of QImage::Format.
     */
    quint32 imageFormat{0};

    /**
     * @brief Reserved for future extensions.
     *
     * This field is currently unused and should be set to zero.
     */
    quint32 reserved{0};

    /**
     * @brief Size of the raw image data in bytes.
     */
    quint64 imageSize{0};

    /**
     * @brief Indicates whether a presentation timestamp is stored.
     *
     * A non-zero value indicates that @ref presentationTimestampNs
     * contains a valid presentation timestamp. Otherwise, constant frame
     * rate timing should be used.
     */
    quint8 hasPresentationTimestamp{0};

    /**
     * @brief Padding bytes.
     *
     * These bytes preserve the binary layout of the structure and must
     * be initialized to zero.
     */
    quint8 padding[7]{};

    /**
     * @brief Presentation timestamp in nanoseconds.
     */
    qint64 presentationTimestampNs{0};
};

static_assert(std::is_trivially_copyable_v<VideoFrameHeader>,
              "VideoFrameHeader must remain trivially copyable.");

static_assert(sizeof(VideoFrameHeader) == 40, "Unexpected VideoFrameHeader size.");

} // namespace fluvel