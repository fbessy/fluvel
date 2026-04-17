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

static inline uint8_t findMedian(const std::array<int, 256>& histogram, int targetRank)
{
    int sum = 0;

    for (int i = 0; i < 256; ++i)
    {
        sum += histogram[i];
        if (sum >= targetRank)
            return static_cast<uint8_t>(i);
    }

    return 255;
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
        uint8_t* dst = output_.data() + y * output_.stride();

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

void Median::applyPerreault(const ImageView& input, int radius)
{
    assert(input.width() == width_);
    assert(input.height() == height_);
    assert(radius >= 1);

    if (!output_.hasSameLayout(input))
        reset(input);

    const int kernelSize = 2 * radius + 1;
    const int medianRank = 1 + kernelSize * kernelSize / 2;

    int I; // pixel intensity, grey-level or channel value for rgb image

    int x, y; // position of the current pixel

    int rank, m;

    for (int ch = 0; ch < channels_; ch++)
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * width_; I++)
        {
            columnsHisto_[I] = 0;
        }

        // initialization
        for (x = 0; x < width_; x++)
        {
            for (y = 0; y < kernelSize; y++)
            {
                columnsHisto_[256 * x + input.at(x, y, ch)]++;
            }
        }

        // downward moving in the image
        for (y = 0; y < radius + 1; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernelHisto_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernelSize; x++)
            {
                columnsHisto_[256 * x + input.at(x, y + radius + 1, ch)]--;
                columnsHisto_[256 * x + input.at(x, y + radius, ch)]++;
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] += columnsHisto_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= radius)
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while (rank < 1 + kernelSize * (x + 1) / 2)
                    {
                        rank += kernelHisto_[++I];
                    }

                    output_.at(x - radius, y, ch) = (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = radius + 1; x < width_ - radius - 1; x++)
            {
                // to update the column histogram H(x+radius)
                columnsHisto_[256 * (x + radius) + input.at(x + radius, y + radius + 1, ch)]--;
                columnsHisto_[256 * (x + radius) + input.at(x + radius, y + radius, ch)]++;

                // to update the mask histogram from the column histograms H(x+radius)
                // and H(x-radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] += columnsHisto_[256 * (x + radius) + I] -
                                       columnsHisto_[256 * (x - radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < medianRank)
                {
                    rank += kernelHisto_[++I];
                }

                output_.at(x, y, ch) = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for (; x < width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+radius)
                // and H(x-radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] -= columnsHisto_[256 * (x - radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < 1 + kernelSize * (kernelSize - m) / 2)
                {
                    rank += kernelHisto_[++I];
                }

                output_.at(x, y, ch) = (unsigned char)(I);
                m++;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * width_; I++)
        {
            columnsHisto_[I] = 0;
        }

        // initialization
        for (x = 0; x < width_; x++)
        {
            for (y = 0; y < kernelSize; y++)
            {
                columnsHisto_[256 * x + input.at(x, y, ch)]++;
            }
        }

        // downward moving in the image
        for (y = radius + 1; y <= height_ - radius - 2; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernelHisto_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernelSize; x++)
            {
                columnsHisto_[256 * x + input.at(x, y - radius - 1, ch)]--;
                columnsHisto_[256 * x + input.at(x, y + radius, ch)]++;
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] += columnsHisto_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= radius)
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while (rank < 1 + kernelSize * (x + 1) / 2)
                    {
                        rank += kernelHisto_[++I];
                    }

                    output_.at(x - radius, y, ch) = (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = radius + 1; x < width_ - radius - 1; x++)
            {
                // to update the column histogram H(x+radius)
                columnsHisto_[256 * (x + radius) + input.at(x + radius, y - radius - 1, ch)]--;
                columnsHisto_[256 * (x + radius) + input.at(x + radius, y + radius, ch)]++;

                // to update the mask histogram from the column histograms H(x+radius)
                // and H(x-radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] += columnsHisto_[256 * (x + radius) + I] -
                                       columnsHisto_[256 * (x - radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < medianRank)
                {
                    rank += kernelHisto_[++I];
                }

                output_.at(x, y, ch) = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for (; x < width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+radius)
                // and H(x-radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] -= columnsHisto_[256 * (x - radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < 1 + kernelSize * (kernelSize - m) / 2)
                {
                    rank += kernelHisto_[++I];
                }
                output_.at(x, y, ch) = (unsigned char)(I);
                m++;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for (y = height_ - radius - 1; y < height_; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernelHisto_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernelSize; x++)
            {
                columnsHisto_[256 * x + input.at(x, y - radius - 1, ch)]--;
                columnsHisto_[256 * x + input.at(x, y - radius, ch)]++;
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] += columnsHisto_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= radius)
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while (rank < 1 + kernelSize * (x + 1) / 2)
                    {
                        rank += kernelHisto_[++I];
                    }
                    output_.at(x - radius, y, ch) = (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = radius + 1; x < width_ - radius - 1; x++)
            {
                // to update the column histogram H(x+radius)
                columnsHisto_[256 * (x + radius) + input.at(x + radius, y - radius - 1, ch)]--;
                columnsHisto_[256 * (x + radius) + input.at(x + radius, y - radius, ch)]++;

                // to update the mask histogram from the column histograms H(x+radius)
                // and H(x-radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] += columnsHisto_[256 * (x + radius) + I] -
                                       columnsHisto_[256 * (x - radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < medianRank)
                {
                    rank += kernelHisto_[++I];
                }

                output_.at(x, y, ch) = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for (; x < width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+radius)
                // and H(x-radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernelHisto_[I] -= columnsHisto_[256 * (x - radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < 1 + kernelSize * (kernelSize - m) / 2)
                {
                    rank += kernelHisto_[++I];
                }

                output_.at(x, y, ch) = (unsigned char)(I);
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
