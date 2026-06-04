// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_alpha.hpp"
#include "image_owner.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>

namespace fluvel_ip::filter::morpho
{

namespace detail
{

/**
 * @brief Horizontal pass of the Van Herk algorithm.
 *
 * Computes sliding-window min/max (depending on IsMax) along image rows
 * using a monotonic deque for O(N) complexity.
 *
 * @tparam IsMax If true computes dilation (max), otherwise erosion (min).
 * @param src Pointer to input row data.
 * @param dst Pointer to output row data.
 * @param width Image width.
 * @param channels Number of channels per pixel (1, 3 or 4).
 * @param activeChannels Number of processed channels (1 or 3).
 * @param radius Radius of the structuring element.
 */
template <bool IsMax>
void vanHerkHorizontal(const uint8_t* src, uint8_t* dst, int width, int channels,
                       int activeChannels, int radius)
{
    const int kernelSize = 2 * radius + 1;

    for (int ch = 0; ch < activeChannels; ++ch)
    {
        std::deque<int> dq;

        for (int i = 0; i < width; ++i)
        {
            // Remove indices outside the sliding window
            if (!dq.empty() && dq.front() <= i - kernelSize)
                dq.pop_front();

            const int idx = i * channels + ch;

            // Maintain monotonic deque
            while (!dq.empty())
            {
                const uint8_t a = src[dq.back() * channels + ch];
                const uint8_t b = src[idx];

                if constexpr (IsMax)
                {
                    if (a <= b)
                        dq.pop_back();
                    else
                        break;
                }
                else
                {
                    if (a >= b)
                        dq.pop_back();
                    else
                        break;
                }
            }

            dq.push_back(i);

            // Write centered result
            if (i >= radius)
            {
                const int out_i = i - radius;
                dst[out_i * channels + ch] = src[dq.front() * channels + ch];
            }
        }

        // Handle right border (tail)
        for (int i = width - radius; i < width; ++i)
        {
            dst[i * channels + ch] = src[dq.front() * channels + ch];
        }
    }
}

/**
 * @brief Vertical pass of the Van Herk algorithm.
 *
 * Applies the same sliding-window logic as the horizontal pass,
 * but along image columns.
 *
 * @tparam IsMax If true computes dilation (max), otherwise erosion (min).
 * @param buffer Intermediate buffer from horizontal pass.
 * @param output Output image.
 * @param width Image width.
 * @param height Image height.
 * @param channels Number of channels per pixel (1, 3 or 4).
 * @param activeChannels Number of processed channels (1 or 3).
 * @param radius Radius of the structuring element.
 */
template <bool IsMax>
void vanHerkVertical(const ImageOwner& buffer, ImageOwner& output, int width, int height,
                     int channels, int activeChannels, int radius)
{
    const int kernelSize = 2 * radius + 1;

    for (int x = 0; x < width; ++x)
    {
        for (int ch = 0; ch < activeChannels; ++ch)
        {
            std::deque<int> dq;

            for (int y = 0; y < height; ++y)
            {
                if (!dq.empty() && dq.front() <= y - kernelSize)
                    dq.pop_front();

                const uint8_t* row = buffer.rowPtr(y);
                const int idx = x * channels + ch;

                while (!dq.empty())
                {
                    const uint8_t* rowBack = buffer.rowPtr(dq.back());

                    const uint8_t a = rowBack[idx];
                    const uint8_t b = row[idx];

                    if constexpr (IsMax)
                    {
                        if (a <= b)
                            dq.pop_back();
                        else
                            break;
                    }
                    else
                    {
                        if (a >= b)
                            dq.pop_back();
                        else
                            break;
                    }
                }

                dq.push_back(y);

                if (y >= radius)
                {
                    const int out_y = y - radius;

                    uint8_t* dst = output.rowPtr(out_y);
                    const uint8_t* rowFront = buffer.rowPtr(dq.front());

                    dst[idx] = rowFront[idx];
                }
            }

            // Handle bottom border (tail)
            for (int y = height - radius; y < height; ++y)
            {
                uint8_t* dst = output.rowPtr(y);
                const uint8_t* rowFront = buffer.rowPtr(dq.front());

                const int idx = x * channels + ch;
                dst[idx] = rowFront[idx];
            }
        }
    }
}

} // namespace detail

/**
 * @brief Morphological filter using the Van Herk algorithm.
 *
 * Computes grayscale dilation (max) or erosion (min) with a square
 * structuring element of size (2 * radius + 1).
 *
 * This implementation achieves O(N) complexity per row/column using
 * a sliding-window monotonic deque.
 *
 * The algorithm is applied in two separable passes:
 * - horizontal
 * - vertical
 *
 * @tparam IsMax If true computes dilation, otherwise erosion.
 * @param input Input image.
 * @param output Output image (must have same layout as input).
 * @param radius Radius of the structuring element.
 */
template <bool IsMax>
void vanHerk(const ImageView& input, ImageOwner& output, int radius)
{
    output.ensureLike(input);

    assert(output.width() == input.width());
    assert(output.height() == input.height());
    assert(output.format() == input.format());
    assert(radius >= 1);

    const int w = input.width();
    const int h = input.height();

    // Degenerate case: no spatial neighborhood
    if (w == 1 || h == 1)
    {
        output.copyFrom(input);
        return;
    }

    const int channels = input.channels();
    const int activeChannels = std::min(channels, 3);

    // Clamp radius to valid range
    const int maxRadius = std::min((w - 1) / 2, (h - 1) / 2);
    radius = std::min(radius, maxRadius);

    // Intermediate buffer
    ImageOwner buffer(w, h, input.format());

    // Horizontal pass
    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = buffer.rowPtr(y);

        detail::vanHerkHorizontal<IsMax>(src, dst, w, channels, activeChannels, radius);
    }

    // Vertical pass
    detail::vanHerkVertical<IsMax>(buffer, output, w, h, channels, activeChannels, radius);

    copyAlpha(input, output);
}

} // namespace fluvel_ip::filter::morpho
