// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_worker.hpp"

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QImage>
#include <QObject>
#include <QString>
#include <QVector>

namespace fluvel
{

/**
 * @brief High-level controller for image-based processing.
 *
 * ImageController acts as the bridge between the user interface and
 * the image-processing backend.
 *
 * Responsibilities include:
 * - loading images from disk
 * - applying preprocessing operations (downscaling, filtering)
 * - configuring and driving the active contour worker
 * - forwarding processed images and contour updates to the UI
 * - managing the processing lifecycle (restart, pause, step, convergence)
 *
 * This class is intended to run in the Qt main thread and coordinates
 * all image-processing operations for a static image session.
 *
 * @note This class is intended to run in the Qt main thread.
 */
class ImageController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct an ImageController with initial session settings.
     */
    ImageController(const ImageSessionSettings& session, QObject* parent);

    /**
     * @brief Load an image from disk.
     *
     * @param path Path to the image file.
     */
    void loadImage(const QString& path);

    /**
     * @brief Update processing configuration.
     */
    void onImageSettingsChanged(const fluvel::ImageSessionSettings& session);

    /**
     * @brief Update display configuration.
     */
    void onImageDisplaySettingsChanged(const fluvel::DisplayConfig& display);

    /**
     * @brief Restart processing from initial state.
     */
    void restart();

    /**
     * @brief Toggle pause/resume of the algorithm.
     */
    void togglePause();

    /**
     * @brief Execute a single iteration of the algorithm.
     */
    void step();

    /**
     * @brief Run the algorithm until convergence.
     */
    void converge();

signals:
    /// Emitted when an error occurs (e.g. failed image load).
    void errorOccurred(const QString& msg);

    /// Emitted when a warning occurs.
    void warningOccurred(const QString& msg);

    /// Emitted when an image is successfully opened.
    void imageOpened(const QString& path);

    /// Emitted when the input image is ready.
    void inputImageReady(const QImage& inputImage);

    /// Emitted when the displayed image is updated.
    void displayedImageReady(const QImage& displayed);

    /// Emitted when contours are updated (UI representation).
    void contourUpdated(const QVector<QPointF>& outerContour, const QVector<QPointF>& innerContour);

    /// Emitted when worker state changes.
    void stateChanged(fluvel::WorkerState state);

    /// Emitted with textual diagnostics information.
    void textDiagnosticsUpdated(QString string);

    /// Request to clear displayed contours.
    void clearContourRequested();

private:
    /// Handle processed image from worker.
    void onProcessedImageReady(const QImage& processed);

    /// Handle contour update from worker (core representation).
    void onContourUpdated(const fluvel_ip::Contour& outerContour,
                          const fluvel_ip::Contour& innerContour);

    /// Handle worker state change.
    void onStateChanged(fluvel::WorkerState state);

    /// Handle diagnostics update from worker.
    void onDiagnosticsUpdated(const fluvel_ip::ContourDiagnostics& diag);

    /// Apply downscaling to the input image if enabled.
    void downscaleImage();

    /// Reinitialize the worker with current configuration.
    void reinitializeWorker();

    /// Refresh displayed image and overlays.
    void refreshView();

    /// Original input image.
    QImage inputImage_;

    /// Downscaled image used for processing.
    QImage downscaledImage_;

    /// Last processed image.
    QImage processedImage_;

    /// Display configuration.
    DisplayConfig displayConfig_{};

    /// Processing configuration.
    ImageComputeConfig computeConfig_{};

    /// Active contour processing worker.
    ActiveContourWorker activeContourWorker_;
};

} // namespace fluvel
