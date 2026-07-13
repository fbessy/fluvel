// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_rendering_utils.hpp"

#include "application_settings_types.hpp"
#include "color_adapters.hpp"
#include "frame_pipeline.hpp"

#include <QPainter>

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

void drawContourOverlay(QImage& image, const DisplayFrame& frame,
                        const DisplayConfig& displayConfig, const DownscaleParams& downscaleParams)
{
    if (image.isNull() ||
        (!displayConfig.outerContourVisible && !displayConfig.innerContourVisible))
    {
        return;
    }

    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setRenderHint(QPainter::TextAntialiasing, false);

    painter.setBrush(Qt::NoBrush);

    const qreal factor = contourScaleFactor(displayConfig, downscaleParams);

    painter.scale(factor, factor);

    QPen pen;
    pen.setWidthF(1.1);
    pen.setCosmetic(false);

    if (displayConfig.outerContourVisible)
    {
        pen.setColor(toQColor(displayConfig.outerContourColor));
        painter.setPen(pen);

        painter.drawPoints(frame.outerContour.data(), static_cast<int>(frame.outerContour.size()));
    }

    if (displayConfig.innerContourVisible)
    {
        pen.setColor(toQColor(displayConfig.innerContourColor));
        painter.setPen(pen);

        painter.drawPoints(frame.innerContour.data(), static_cast<int>(frame.innerContour.size()));
    }
}

} // namespace fluvel::frame_rendering_utils