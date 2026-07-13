// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QImage>

namespace fluvel
{

struct DisplayFrame;
struct DisplayConfig;
struct DownscaleParams;

namespace frame_rendering_utils
{

/**
 * @brief Draws visible contour overlays onto an image.
 *
 * Renders the outer and inner contours from the specified display frame
 * directly onto the target image.
 *
 * Contours are expressed in processing image coordinates and are scaled
 * to the represented image space according to the current display and
 * downscale configuration.
 *
 * Only contours marked as visible in the display configuration are rendered.
 * Contour colors are also taken from the display configuration.
 *
 * The rendering parameters match those used by the interactive image viewer.
 *
 * @param image Image onto which the contours are rendered.
 * @param frame Display frame containing the contours.
 * @param displayConfig Display configuration controlling contour visibility,
 *        colors and image representation.
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