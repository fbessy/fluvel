// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file image_settings_controller.hpp
 * @brief Controller for editing and previewing image processing settings.
 *
 * This component manages the interactive editing of the initial level-set (phi)
 * and associated processing parameters. It provides a preview pipeline that
 * reflects user changes in real time.
 *
 * Responsibilities:
 * - Manage user-defined shapes (add/subtract)
 * - Maintain and update phi representation
 * - Apply preprocessing (downscale + filters)
 * - Generate preview images for UI
 */

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#include "phi_editor.hpp"
#include "phi_view_model.hpp"
#endif

#include <QObject>

namespace fluvel_app
{

/**
 * @brief Normalized shape description in UI space.
 *
 * Coordinates and dimensions are expressed as percentages relative to the
 * image size. Position is centered.
 */
struct UiShapeInfo
{
    ShapeType shape; ///< Shape type (e.g. circle, rectangle)
    int width;       ///< Width in percent of image size
    int height;      ///< Height in percent of image size
    int x;           ///< Center X position (percent)
    int y;           ///< Center Y position (percent)
};

/**
 * @brief Controller for image settings editing and preview.
 *
 * This class connects UI interactions (shape editing, parameter updates)
 * with the underlying phi editor and processing pipeline.
 *
 * It maintains an editable configuration and provides a preview of the
 * resulting processed image.
 *
 * @note Intended to run in the Qt main thread.
 */
class ImageSettingsController : public QObject
{
    Q_OBJECT

public:
    ImageSettingsController(const ImageSessionSettings& session, QObject* parent);

    /**
     * @brief Set visibility of the settings view.
     *      * When hidden, updates may be deferred to avoid unnecessary processing.
     */
    void setViewVisible(bool v);

    /**
     * @brief Add a shape to the phi mask.
     */
    void addShape(UiShapeInfo uiShape);

    /**
     * @brief Subtract a shape from the phi mask.
     */
    void subtractShape(UiShapeInfo uiShape);

    /**
     * @brief Clear the phi mask.
     */
    void clearPhi();

    /**
     * @brief Update input image used for preview.
     */
    void onInputImageReady(const QImage& inputImage);

    /**
     * @brief Update editable processing configuration.
     */
    void updateEditedConfig(const DownscaleParams& downscaleParams,
                            const fluvel_ip::ProcessingParams& processingParams);

    /**
     * @brief Commit current edits and return resulting phi as image.
     *      * @return Image representing the committed state.
     */
    QImage commit();

    /**
     * @brief Revert edits to last committed state.
     */
    void revert();

    /**
     * @brief Update overlay shape during interactive editing.
     */
    void onUpdateOverlay(fluvel_app::UiShapeInfo uiShape);

    /**
     * @brief Update connectivity mode of the contour.
     */
    void onConnectivityChanged(fluvel_ip::Connectivity c);

    /**
     * @brief Enable or disable interactive editing mode.
     */
    void setInteractiveMode(bool enabled);

signals:
    /// Emitted when the preview image changes.
    void viewChanged(const QImage& imageSettings);

    /// Emitted when processing starts.
    void processingStarted();

    /// Emitted when the filter pipeline completes.
    void filterPipelineProcessed(double elapsedSec);

private:
    /// Internal handler for view updates.
    void onViewChanged(const QImage& imageSettings);

    /**
     * @brief Convert UI shape to image-space shape.
     *      * @param uiShape Shape in normalized UI coordinates.
     * @param targetSize Target image size.
     * @return Shape in image coordinates.
     */
    ShapeInfo computeShapeInfo(const UiShapeInfo& uiShape, const QSize& targetSize);

    /// Apply downscaling to input image.
    void applyDownscale();

    /// Apply processing pipeline to image.
    void applyProcessing();

    /// Refresh preview image.
    void refreshPreview();

    /// Editor for phi (level-set representation).
    std::unique_ptr<PhiEditor> phiEditor_;

    /// View model for phi visualization.
    std::unique_ptr<PhiViewModel> phiViewModel_;

    /// Editable downscale parameters.
    DownscaleParams editedDownscaleParams_;

    /// Editable processing parameters.
    fluvel_ip::ProcessingParams editedProcessingParams_;

    /// Original input image.
    QImage input_;

    /// Downscaled image.
    QImage downscaled_;

    /// Processed preview image.
    QImage processed_;

    /// Whether initialization phase is active.
    bool initializationEnabled_{false};

    /// Whether preview refresh is needed.
    bool needsRefresh_{false};

    /// Whether the view is currently visible.
    bool viewVisible_{false};
};

} // namespace fluvel_app
