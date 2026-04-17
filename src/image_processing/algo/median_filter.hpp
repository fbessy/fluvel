// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"
#include <array>

namespace fluvel_ip::filter
{

void median(const ImageView& input, ImageOwner& output, int radius);
ImageOwner median(const ImageView& input, int radius);

class Median
{
public:
    void reset(const ImageView& input);
    void apply(const ImageView& input, int radius);

    ImageView outputView() const;
    const ImageOwner& output() const;
    ImageOwner& outputRef();

private:
    void applyPerreault(const ImageView& input, int radius);
    void applyNaive(const ImageView& input, int radius);

private:
    ImageOwner buffer_;
    ImageOwner output_;

    std::vector<int> columnsHisto_; // size = width * 256
    std::array<int, 256> kernelHisto_{};

    int width_{0};
    int height_{0};
    int channels_{0};
};

} // namespace fluvel_ip::filter
