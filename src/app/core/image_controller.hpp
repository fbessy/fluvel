// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file image_controller.hpp
 * @brief Controller for static image processing workflow.
 *
 * This component manages the lifecycle of image-based processing:
 * - loading images from disk
 * - applying preprocessing (downscaling, filters)
 * - driving the active contour algorithm
 * - updating the UI with processed images and contours
 *
 * It acts as the bridge between UI interactions and the processing backend.
 */

#pragma once

#include "active_contour_worker.hpp"

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QImage>
#include <QObject>
#include <QString>
#include <QVector>

namespace fluvel_app
{

/**
 * @brief High-level controller for image processing.
 *
 * Responsibilities:
 * - Load and manage input images
 * - Configure and control the active contour worker
 * - Handle preprocessing (e.g. downscaling)
 * - Convert and forward results to the UI layer
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
     *      * @param path Path to the image file.
     */
    void loadImage(const QString& path);

    /**
     * @brief Update processing configuration.
     */
    void onImageSettingsChanged(const fluvel_app::ImageSessionSettings& session);

    /**
     * @brief Update display configuration.
     */
    void onImageDisplaySettingsChanged(const fluvel_app::DisplayConfig& display);

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
    void stateChanged(fluvel_app::WorkerState state);

    /// Emitted with textual diagnostics information.
    void textDiagnosticsUpdated(QString string);

    /// Request to clear displayed contours.
    void clearContourRequested();

private:
    /// Handle processed image from worker.
    void onProcessedImageReady(const QImage& processed);

    /// Handle contour update from worker (core representation).
    void onContourUpdated(const fluvel_ip::ExportedContour& outerContour,
                          const fluvel_ip::ExportedContour& innerContour);

    /// Handle worker state change.
    void onStateChanged(fluvel_app::WorkerState state);

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

} // namespace fluvel_app
