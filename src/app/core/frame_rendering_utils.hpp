// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point_layer.hpp"

#include <QColor>
#include <QImage>
#include <QVector>

namespace fluvel
{

struct DisplayFrame;
struct DisplayConfig;
struct DownscaleParams;

namespace frame_rendering_utils
{

/**
 * @brief Draws styled layers of points onto an image.
 *
 * The rendering parameters match those used by the interactive image viewer.
 * Hidden layers are skipped.
 *
 * @param image Image onto which the layers are rendered.
 * @param layers Layers of points, in processing image coordinates.
 * @param scale Scale applied to the point coordinates.
 */
void drawLayers(QImage& image, const QVector<PointLayer>& layers, qreal scale);

/**
 * @brief Draws the layers of a display frame onto an image.
 *
 * Layers carry their own style (color, visibility), set by the controller.
 * They are expressed in processing image coordinates and are scaled to the
 * represented image space according to the current display and downscale
 * configuration.
 *
 * @param image Image onto which the layers are rendered.
 * @param frame Display frame containing the layers.
 * @param displayConfig Display configuration (image representation).
 * @param downscaleParams Processing downscale parameters.
 */
void drawContourOverlay(QImage& image, const DisplayFrame& frame,
                        const DisplayConfig& displayConfig, const DownscaleParams& downscaleParams);

/**
 * @brief Computes the contour scale factor for the represented image.
 *
 * Contours are expressed in processing image coordinates. When the source
 * image is displayed while downscaled processing is enabled, contour
 * coordinates must be scaled back to the source image space.
 *
 * @param displayConfig Display configuration.
 * @param downscaleParams Processing downscale parameters.
 *
 * @return Contour scale factor.
 */
[[nodiscard]] qreal contourScaleFactor(const DisplayConfig& displayConfig,
                                       const DownscaleParams& downscaleParams);

} // namespace frame_rendering_utils

} // namespace fluvel