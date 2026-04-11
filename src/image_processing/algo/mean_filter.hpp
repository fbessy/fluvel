// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter
{

void mean(const ImageView& input, ImageOwner& output, int radius);
ImageOwner mean(const ImageView& input, int radius);

} // namespace fluvel_ip::filter
