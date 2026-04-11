// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "mean_filter.hpp"
#include "mean_filter_3x3.hpp"
#include "mean_filter_sliding.hpp"

namespace fluvel_ip::filter
{

void mean(const ImageView& input, ImageOwner& output, int radius)
{
    if (radius >= 2)
        meanSliding(input, output, radius);
    else
        mean3x3(input, output);
}

ImageOwner mean(const ImageView& input, int radius)
{
    ImageOwner out;
    mean(input, out, radius);
    return out;
}

} // namespace fluvel_ip::filter
