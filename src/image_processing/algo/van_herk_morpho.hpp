// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"

#include <cstdint>
#include <deque>

namespace fluvel_ip::filter::morpho
{

namespace detail
{

template <bool IsMax>
void vanHerkHorizontal(const uint8_t* src, uint8_t* dst, int width, int channels, int radius)
{
    const int K = 2 * radius + 1;

    for (int ch = 0; ch < channels; ++ch)
    {
        std::deque<int> dq;

        for (int i = 0; i < width; ++i)
        {
            // remove out-of-window
            if (!dq.empty() && dq.front() <= i - K)
                dq.pop_front();

            int idx = i * channels + ch;

            // maintain monotonic deque
            while (!dq.empty())
            {
                uint8_t a = src[dq.back() * channels + ch];
                uint8_t b = src[idx];

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

            // write result (centered window)
            if (i >= radius)
            {
                int out_i = i - radius;
                dst[out_i * channels + ch] = src[dq.front() * channels + ch];
            }
        }

        // tail (bord droit)
        for (int i = width - radius; i < width; ++i)
        {
            dst[i * channels + ch] = src[dq.front() * channels + ch];
        }
    }
}

template <bool IsMax>
void vanHerkVertical(const ImageOwner& buffer, ImageOwner& output, int width, int height,
                     int channels, int radius)
{
    const int K = 2 * radius + 1;

    for (int x = 0; x < width; ++x)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            std::deque<int> dq;

            for (int y = 0; y < height; ++y)
            {
                if (!dq.empty() && dq.front() <= y - K)
                    dq.pop_front();

                const uint8_t* row = buffer.rowPtr(y);
                int idx = x * channels + ch;

                while (!dq.empty())
                {
                    const uint8_t* rowBack = buffer.rowPtr(dq.back());

                    uint8_t a = rowBack[idx];
                    uint8_t b = row[idx];

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
                    int out_y = y - radius;

                    uint8_t* dst = output.rowPtr(out_y);
                    const uint8_t* rowFront = buffer.rowPtr(dq.front());

                    dst[idx] = rowFront[idx];
                }
            }

            // tail
            for (int y = height - radius; y < height; ++y)
            {
                uint8_t* dst = output.rowPtr(y);
                const uint8_t* rowFront = buffer.rowPtr(dq.front());

                int idx = x * channels + ch;
                dst[idx] = rowFront[idx];
            }
        }
    }
}

} // namespace detail

template <bool IsMax>
void vanHerk(const ImageView& input, ImageOwner& output, int radius)
{
    assert(output.width() == input.width());
    assert(output.height() == input.height());
    assert(output.format() == input.format());

    const int w = input.width();
    const int h = input.height();
    const int c = input.channels();

    // buffer intermédiaire
    ImageOwner buffer(w, h, input.format());

    // =========================
    // PASS 1 — Horizontal
    // =========================
    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = buffer.rowPtr(y);

        detail::vanHerkHorizontal<IsMax>(src, dst, w, c, radius);
    }

    // =========================
    // PASS 2 — Vertical
    // =========================
    detail::vanHerkVertical<IsMax>(buffer, output, w, h, c, radius);
}

} // namespace fluvel_ip::filter::morpho
