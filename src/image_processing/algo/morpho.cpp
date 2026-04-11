// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "morpho.hpp"
#include "naive_morpho.hpp"
#include "pixel_wise.hpp"
#include "van_herk_morpho.hpp"

namespace fluvel_ip::filter::morpho
{

void max(const ImageView& input, ImageOwner& output, int radius)
{
    if (radius >= 2)
        vanHerk<true>(input, output, radius);
    else
        naiveMorpho<true>(input, output, radius);
}

ImageOwner max(const ImageView& input, int radius)
{
    ImageOwner out;
    max(input, out, radius);
    return out;
}

void min(const ImageView& input, ImageOwner& output, int radius)
{
    if (radius >= 2)
        vanHerk<false>(input, output, radius);
    else
        naiveMorpho<false>(input, output, radius);
}

ImageOwner min(const ImageView& input, int radius)
{
    ImageOwner out;
    min(input, out, radius);
    return out;
}

void opening(const ImageView& input, ImageOwner& output, int radius)
{
    ImageOwner tmp(input.width(), input.height(), input.format());

    min(input, tmp, radius);
    max(tmp.view(), output, radius);
}

ImageOwner opening(const ImageView& input, int radius)
{
    ImageOwner out;
    opening(input, out, radius);
    return out;
}

void closing(const ImageView& input, ImageOwner& output, int radius)
{
    ImageOwner tmp(input.width(), input.height(), input.format());

    max(input, tmp, radius);
    min(tmp.view(), output, radius);
}

ImageOwner closing(const ImageView& input, int radius)
{
    ImageOwner out;
    closing(input, out, radius);
    return out;
}

void topHat(const ImageView& input, ImageOwner& output, int radius)
{
    ImageOwner tmp(input.width(), input.height(), input.format());
    opening(input, tmp, radius);
    pixelwise::diff(input, tmp.view(), output);
}

ImageOwner topHat(const ImageView& input, int radius)
{
    ImageOwner out;
    topHat(input, out, radius);
    return out;
}

void blackTopHat(const ImageView& input, ImageOwner& output, int radius)
{
    ImageOwner tmp(input.width(), input.height(), input.format());
    closing(input, tmp, radius);
    pixelwise::diff(tmp.view(), input, output);
}

ImageOwner blackTopHat(const ImageView& input, int radius)
{
    ImageOwner out;
    blackTopHat(input, out, radius);
    return out;
}

} // namespace fluvel_ip::filter::morpho
