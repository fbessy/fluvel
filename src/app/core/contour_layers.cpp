// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "contour_layers.hpp"
#include "application_settings_types.hpp"
#include "color_adapters.hpp"

namespace fluvel::contour_layers
{

QVector<PointLayer> make(const QVector<QPointF>& outer, const QVector<QPointF>& inner,
                         const DisplayConfig& displayConfig)
{
    QVector<PointLayer> layers(2);

    layers[kOuter] = {outer, toQColor(displayConfig.outerContourColor),
                      displayConfig.outerContourVisible};

    layers[kInner] = {inner, toQColor(displayConfig.innerContourColor),
                      displayConfig.innerContourVisible};

    return layers;
}

QVector<PointLayer> restyle(const QVector<PointLayer>& layers, const DisplayConfig& displayConfig)
{
    const QVector<QPointF> outer = layers.size() > kOuter ? layers[kOuter].points : QVector<QPointF>{};
    const QVector<QPointF> inner = layers.size() > kInner ? layers[kInner].points : QVector<QPointF>{};

    return make(outer, inner, displayConfig);
}

} // namespace fluvel::contour_layers
