// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter::morpho
{

void max(const ImageView& input, ImageOwner& output, int radius);
ImageOwner max(const ImageView& input, int radius);

void min(const ImageView& input, ImageOwner& output, int radius);
ImageOwner min(const ImageView& input, int radius);

void opening(const ImageView& input, ImageOwner& output, int radius);
ImageOwner opening(const ImageView& input, int radius);

void closing(const ImageView& input, ImageOwner& output, int radius);
ImageOwner closing(const ImageView& input, int radius);

void topHat(const ImageView& input, ImageOwner& output, int radius);
ImageOwner topHat(const ImageView& input, int radius);

void blackTopHat(const ImageView& input, ImageOwner& output, int radius);
ImageOwner blackTopHat(const ImageView& input, int radius);

} // namespace fluvel_ip::filter::morpho
