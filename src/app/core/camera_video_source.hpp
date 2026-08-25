// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_types.hpp"

#include <QCamera>
#include <QCameraFormat>
#include <QMediaCaptureSession>
#include <QObject>

class QVideoSink;

namespace fluvel
{

/**
 * @brief Manages a camera video source.
 *
 * CameraVideoSource encapsulates the Qt Multimedia camera backend used to
 * configure, start and stop a camera source.
 *
 * The class owns the camera instance and its capture session, while the
 * video sink is supplied by the caller and is not owned by this class.
 *
 * Camera-specific errors are reported through the error() signal. The caller
 * remains responsible for deciding how these errors affect the application
 * state.
 */
class CameraVideoSource : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a camera video source.
     *
     * @param parent Optional QObject parent.
     */
    explicit CameraVideoSource(QObject* parent = nullptr);

    /**
     * @brief Destroys the camera video source.
     */
    ~CameraVideoSource() override;

    /**
     * @brief Starts the requested camera.
     *
     * The camera is selected and configured according to the specified
     * configuration. Starting is asynchronous; backend errors are reported
     * through error().
     *
     * @param config Camera source configuration.
     *
     * @return true if the requested camera was found and the start operation
     *         was initiated, false otherwise.
     */
    bool start(const CameraConfig& config);

    /**
     * @brief Stops the active camera.
     *
     * If no camera is currently active, this function has no effect.
     */
    void stop();

    /**
     * @brief Sets the video sink receiving frames from the camera.
     *
     * The sink is not owned by CameraVideoSource and must remain valid while
     * it is assigned to the capture session.
     *
     * @param sink Video sink to receive camera frames, or nullptr to detach it.
     */
    void setVideoSink(QVideoSink* sink);

    /**
     * @brief Returns information about the active camera.
     *
     * @return Runtime camera information, or an empty CameraInfo if no
     *         camera is currently active.
     */
    [[nodiscard]] CameraInfo cameraInfo() const;

    /**
     * @brief Returns whether a camera is currently active.
     *
     * @return true if a camera instance is active, false otherwise.
     */
    [[nodiscard]] bool isActive() const noexcept;

signals:
    /**
     * @brief Reports an error from the camera backend.
     *
     * @param error Error reported by Qt Multimedia.
     * @param errorString Human-readable error description.
     */
    void error(QCamera::Error error, const QString& errorString);

private:
    QCamera* camera_{nullptr};
    QMediaCaptureSession captureSession_;
};

} // namespace fluvel