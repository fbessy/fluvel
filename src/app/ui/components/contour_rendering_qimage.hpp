// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "contour_data.hpp"

#include <QImage>
#include <QRgb>

namespace fluvel_app
{

void draw_list_to_img(const std::vector<fluvel_ip::ContourPoint>& list,
                      const fluvel_ip::Rgb_uc& color, QImage& img);

void draw_upscale_list(const std::vector<fluvel_ip::ContourPoint>& list,
                       const fluvel_ip::Rgb_uc& color, int upscale_factor, QImage& img_rgb32);
} // namespace fluvel_app
