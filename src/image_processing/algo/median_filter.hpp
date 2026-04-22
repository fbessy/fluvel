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

    static constexpr int kHistogramSize = 256;

    template <typename Container>
    void clearHistogram(Container& histo)
    {
        std::fill(histo.begin(), histo.end(), 0);
    }

    void initCurrentMedian()
    {
        currentMedian_ = kHistogramSize / 2;
    }

    void accumulateColumn(int colIndex);
    void removeColumn(int colIndex);
    void updateKernel(int addColIndex, int removeColIndex);
    uint8_t findMedian(int targetRank);

    void initColumnsHisto(const ImageView& input, int ch, int kernelSize);
    void updateColumnsHisto(int colIndex, uint8_t valRemove, uint8_t valAdd);

    ImageOwner buffer_;
    ImageOwner output_;

    std::vector<int> columnsHisto_; // size = width * 256
    std::array<int, kHistogramSize> kernelHisto_{};

    int currentMedian_{kHistogramSize / 2};

    int width_{0};
    int height_{0};
    int channels_{0};
};

} // namespace fluvel_ip::filter
