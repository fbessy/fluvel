// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file recording_session.hpp
 * @brief Video recording session management.
 */

#pragma once

#include "video_export_settings.hpp"
#include "video_exporter.hpp"

#include <optional>

namespace fluvel
{

struct VideoFrame;

/**
 * @brief Manages a video recording session.
 *
 * A recording session coordinates the lifecycle of a video recording and
 * provides a higher-level abstraction over the underlying video exporter.
 *
 * In single-file mode, the session records the entire video into a single
 * output file.
 *
 * In circular mode, the session automatically rotates the recording into
 * successive video segments. When a segment reaches the configured duration,
 * it is finalized and a new segment is started transparently. Older segments
 * may be removed according to the configured retention policy.
 *
 * The recording session is independent of the frame buffering mechanism used
 * by the recorder worker.
 */
class RecordingSession
{
public:
    /**
     * @brief Opens a recording session.
     *
     * @param settings Recording settings.
     * @return @c true on success, @c false otherwise.
     */
    bool open(const VideoExportSettings& settings);

    /**
     * @brief Records a video frame.
     *
     * The frame is written to the current output file. In circular mode,
     * this function automatically starts a new segment whenever the current
     * segment reaches its configured duration.
     *
     * @param frame Frame to record.
     * @return @c true on success, @c false otherwise.
     */
    bool addFrame(const VideoFrame& frame);

    /**
     * @brief Finalizes the recording session.
     *
     * Closes the current output file and releases all resources associated
     * with the recording session.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool close();

private:
    /**
     * @brief Starts a new recording segment.
     *
     * Finalizes the current segment, generates the filename of the next
     * segment and opens it for recording.
     *
     * @return @c true on success, @c false otherwise.
     */
    bool rotateSegment();

private:
    QString segmentFilename(int index) const;
    void removeOldSegments();

    VideoExporter exporter_;
    VideoExportSettings settings_{};

    QString segmentDirectory_;
    std::optional<int64_t> currentSegmentStartNs_;
    int segmentIndex_{1};
    int64_t segmentDurationNs_{0};
};

} // namespace fluvel