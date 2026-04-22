// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "median_filter.hpp"

#include <algorithm>
#include <cassert>

namespace fluvel_ip::filter
{

void median(const ImageView& input, ImageOwner& output, int radius)
{
    Median impl;

    impl.reset(input);
    impl.apply(input, radius);

    std::swap(output, impl.outputRef());
}

ImageOwner median(const ImageView& input, int radius)
{
    ImageOwner out;
    median(input, out, radius);
    return out;
}

void Median::reset(const ImageView& input)
{
    width_ = input.width();
    height_ = input.height();
    channels_ = input.channels();

    buffer_ = ImageOwner::like(input);
    output_ = ImageOwner::like(input);

    columnsHisto_.resize(width_ * 256);
}

void Median::apply(const ImageView& input, int radius)
{
    if (radius >= 3)
        applyPerreault(input, radius);
    else
        applyNaive(input, radius);
}

void Median::applyNaive(const ImageView& input, int radius)
{
    assert(input.width() == width_);
    assert(input.height() == height_);
    assert(radius >= 1);

    if (!output_.hasSameLayout(input))
        reset(input);

    const int k = 2 * radius + 1;
    const int size = k * k;

    std::vector<uint8_t> window;
    window.reserve(size);

    for (int y = 0; y < height_; ++y)
    {
        uint8_t* dst = output_.rowPtr(y);

        for (int x = 0; x < width_; ++x)
        {
            for (int ch = 0; ch < channels_; ++ch)
            {
                window.clear();

                for (int ky = -radius; ky <= radius; ++ky)
                {
                    int yy = std::clamp(y + ky, 0, height_ - 1);
                    const uint8_t* row = input.row(yy);

                    for (int kx = -radius; kx <= radius; ++kx)
                    {
                        int xx = std::clamp(x + kx, 0, width_ - 1);
                        window.push_back(row[xx * channels_ + ch]);
                    }
                }

                std::nth_element(window.begin(), window.begin() + window.size() / 2, window.end());

                dst[x * channels_ + ch] = window[window.size() / 2];
            }
        }
    }
}

uint8_t Median::findMedian(int targetRank)
{
    assert(targetRank > 0);

    int cumulative = 0;

    for (int i = 0; i < kHistogramSize; ++i)
    {
        cumulative += kernelHisto_[i];
        if (cumulative >= targetRank)
            return static_cast<uint8_t>(i);
    }

    return 255;
}

void Median::accumulateColumn(int colIndex)
{
    const int base = kHistogramSize * colIndex;

    for (int i = 0; i < kHistogramSize; ++i)
        kernelHisto_[i] += columnsHisto_[base + i];
}

void Median::removeColumn(int colIndex)
{
    const int base = kHistogramSize * colIndex;

    for (int i = 0; i < kHistogramSize; ++i)
        kernelHisto_[i] -= columnsHisto_[base + i];
}

void Median::updateKernel(int addColIndex, int removeColIndex)
{
    const int addBase = kHistogramSize * addColIndex;
    const int remBase = kHistogramSize * removeColIndex;

    for (int i = 0; i < kHistogramSize; ++i)
        kernelHisto_[i] += columnsHisto_[addBase + i] - columnsHisto_[remBase + i];
}

void Median::initColumnsHisto(const ImageView& input, int ch, int kernelSize)
{
    for (int x = 0; x < width_; ++x)
    {
        int base = kHistogramSize * x;

        for (int y = 0; y < kernelSize; ++y)
        {
            uint8_t val = input.at(x, y, ch);
            columnsHisto_[base + val]++;
        }
    }
}

void Median::updateColumnsHisto(int colIndex, uint8_t valRemove, uint8_t valAdd)
{
    const int base = kHistogramSize * colIndex;

    columnsHisto_[base + valRemove]--;
    columnsHisto_[base + valAdd]++;
}

void Median::applyPerreault(const ImageView& input, int radius)
{
    assert(input.width() == width_);
    assert(input.height() == height_);
    assert(radius >= 1);

    if (!output_.hasSameLayout(input))
        reset(input);

    const int kernelSize = 2 * radius + 1;
    const int medianRank = kernelSize * kernelSize / 2 + 1;

    for (int ch = 0; ch < channels_; ch++)
    {
        // ================= TOP =================
        clearHistogram(columnsHisto_);

        initColumnsHisto(input, ch, kernelSize);

        for (int y = 0; y < radius + 1; ++y)
        {
            clearHistogram(kernelHisto_);

            for (int x = 0; x < kernelSize; ++x)
            {
                uint8_t valRemove = input.at(x, y + radius + 1, ch);
                uint8_t valAdd = input.at(x, y + radius, ch);

                updateColumnsHisto(x, valRemove, valAdd);

                accumulateColumn(x);

                if (x >= radius)
                {
                    const int effectiveWidth = x + 1;
                    const int pixelCount = kernelSize * effectiveWidth;
                    const int rank = pixelCount / 2 + 1;

                    uint8_t val = findMedian(rank);
                    output_.at(x - radius, y, ch) = val;
                }
            }

            for (int x = radius + 1; x < width_ - radius - 1; ++x)
            {
                int col = x + radius;

                uint8_t valRemove = input.at(col, y + radius + 1, ch);
                uint8_t valAdd = input.at(col, y + radius, ch);

                updateColumnsHisto(col, valRemove, valAdd);

                updateKernel(col, x - radius - 1);

                uint8_t val = findMedian(medianRank);
                output_.at(x, y, ch) = val;
            }

            int m = 1;
            for (int x = width_ - radius - 1; x < width_; ++x)
            {
                removeColumn(x - radius - 1);

                const int effectiveWidth = kernelSize - m;
                const int pixelCount = kernelSize * effectiveWidth;
                const int rank = pixelCount / 2 + 1;

                uint8_t val = findMedian(rank);
                output_.at(x, y, ch) = val;
                m++;
            }
        }

        // ================= MIDDLE =================
        clearHistogram(columnsHisto_);

        initColumnsHisto(input, ch, kernelSize);

        for (int y = radius + 1; y < height_ - radius - 1; ++y)
        {
            clearHistogram(kernelHisto_);

            for (int x = 0; x < kernelSize; ++x)
            {
                uint8_t valRemove = input.at(x, y - radius - 1, ch);
                uint8_t valAdd = input.at(x, y + radius, ch);

                updateColumnsHisto(x, valRemove, valAdd);

                accumulateColumn(x);

                if (x >= radius)
                {
                    const int effectiveWidth = x + 1;
                    const int pixelCount = kernelSize * effectiveWidth;
                    const int rank = pixelCount / 2 + 1;

                    uint8_t val = findMedian(rank);
                    output_.at(x - radius, y, ch) = val;
                }
            }

            for (int x = radius + 1; x < width_ - radius - 1; ++x)
            {
                int col = x + radius;

                uint8_t valRemove = input.at(col, y - radius - 1, ch);
                uint8_t valAdd = input.at(col, y + radius, ch);

                updateColumnsHisto(col, valRemove, valAdd);

                updateKernel(col, x - radius - 1);

                uint8_t val = findMedian(medianRank);
                output_.at(x, y, ch) = val;
            }

            int m = 1;
            for (int x = width_ - radius - 1; x < width_; ++x)
            {
                removeColumn(x - radius - 1);

                const int effectiveWidth = kernelSize - m;
                const int pixelCount = kernelSize * effectiveWidth;
                const int rank = pixelCount / 2 + 1;

                uint8_t val = findMedian(rank);
                output_.at(x, y, ch) = val;
                m++;
            }
        }

        // ================= BOTTOM =================
        for (int y = height_ - radius - 1; y < height_; ++y)
        {
            clearHistogram(kernelHisto_);

            for (int x = 0; x < kernelSize; ++x)
            {
                uint8_t valRemove = input.at(x, y - radius - 1, ch);
                uint8_t valAdd = input.at(x, y - radius, ch);

                updateColumnsHisto(x, valRemove, valAdd);

                accumulateColumn(x);

                if (x >= radius)
                {
                    const int effectiveWidth = x + 1;
                    const int pixelCount = kernelSize * effectiveWidth;
                    const int rank = pixelCount / 2 + 1;

                    uint8_t val = findMedian(rank);
                    output_.at(x - radius, y, ch) = val;
                }
            }

            for (int x = radius + 1; x < width_ - radius - 1; ++x)
            {
                int col = x + radius;
                uint8_t valRemove = input.at(col, y - radius - 1, ch);
                uint8_t valAdd = input.at(col, y - radius, ch);

                updateColumnsHisto(col, valRemove, valAdd);

                updateKernel(col, x - radius - 1);

                uint8_t val = findMedian(medianRank);
                output_.at(x, y, ch) = val;
            }

            int m = 1;
            for (int x = width_ - radius - 1; x < width_; ++x)
            {
                removeColumn(x - radius - 1);

                const int effectiveWidth = kernelSize - m;
                const int pixelCount = kernelSize * effectiveWidth;
                const int rank = pixelCount / 2 + 1;

                uint8_t val = findMedian(rank);
                output_.at(x, y, ch) = val;
                m++;
            }
        }
    }
}

ImageView Median::outputView() const
{
    return output_.view();
}

const ImageOwner& Median::output() const
{
    return output_;
}

ImageOwner& Median::outputRef()
{
    return output_;
}

} // namespace fluvel_ip::filter
