// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_rendering_utils.hpp"

#include "application_settings_types.hpp"
#include "frame_pipeline.hpp"

#include <QPainter>

#include <algorithm>

namespace fluvel::frame_rendering_utils
{

qreal contourScaleFactor(const DisplayConfig& displayConfig, const DownscaleParams& downscaleParams)
{
    if (downscaleParams.downscaleEnabled && displayConfig.displayMode == ImageDisplayMode::Source)
    {
        return static_cast<qreal>(downscaleParams.downscaleFactor);
    }

    return 1.0;
}

void drawLayers(QImage& image, const QVector<PointLayer>& layers, qreal scale)
{
    if (image.isNull())
        return;

    const bool anyVisible = std::any_of(layers.begin(), layers.end(),
                                        [](const PointLayer& layer) { return layer.visible; });

    if (!anyVisible)
        return;

    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setRenderHint(QPainter::TextAntialiasing, false);

    painter.setBrush(Qt::NoBrush);

    painter.scale(scale, scale);

    QPen pen;
    pen.setWidthF(1.1);
    pen.setCosmetic(false);

    for (const PointLayer& layer : layers)
    {
        if (!layer.visible)
            continue;

        pen.setColor(layer.color);
        painter.setPen(pen);

        painter.drawPoints(layer.points.data(), static_cast<int>(layer.points.size()));
    }
}

void drawContourOverlay(QImage& image, const DisplayFrame& frame,
                        const DisplayConfig& displayConfig, const DownscaleParams& downscaleParams)
{
    drawLayers(image, frame.layers, contourScaleFactor(displayConfig, downscaleParams));
}

} // namespace fluvel::frame_rendering_utils