// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"
#include <algorithm>
#include <array>

namespace fluvel_ip::filter
{

/**
 * @brief Apply a median filter with a preallocated output buffer.
 *
 * This function applies a median filter of radius @p radius to the input image.
 * The implementation may internally select an optimized strategy depending
 * on the radius (e.g. naive or Perreault histogram-based algorithm).
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param radius Radius of the filter (kernel size = 2 * radius + 1).
 */
void median(const ImageView& input, ImageOwner& output, int radius);

/**
 * @brief Apply a median filter and return a new image.
 *
 * Convenience wrapper allocating a new output image.
 *
 * @param input Input image view.
 * @param radius Radius of the filter (kernel size = 2 * radius + 1).
 * @return Filtered image.
 */
ImageOwner median(const ImageView& input, int radius);

/**
 * @brief Stateful median filter implementation.
 *
 * This class provides an efficient median filter with two strategies:
 *
 * - Naive implementation for small kernels
 * - Perreault's histogram-based algorithm for larger kernels (O(N))
 *
 * Internal buffers and histograms are reused across calls, making it suitable
 * for repeated processing (e.g. video streams or pipelines).
 */
class Median
{
public:
    /**
     * @brief Initialize internal buffers and histograms.
     *
     * Allocates or resizes buffers based on the input image layout.
     *
     * @param input Input image view.
     */
    void reset(const ImageView& input);

    /**
     * @brief Apply the median filter.
     *
     * Selects the appropriate implementation depending on the radius.
     *
     * @param input Input image view.
     * @param radius Radius of the filter (kernel size = 2 * radius + 1).
     */
    void apply(const ImageView& input, int radius);

    /**
     * @brief Get a non-owning view of the result image.
     *
     * @return ImageView referencing the internal output buffer.
     */
    ImageView outputView() const;

    /**
     * @brief Get the result image with ownership.
     *
     * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output() const;

    /**
     * @brief Get a mutable reference to the output image.
     *
     * Useful for move or swap-based APIs.
     *
     * @return Reference to the internal ImageOwner.
     */
    ImageOwner& outputRef();

private:
    /**
     * @brief Apply Perreault's fast median filter.
     *
     * Uses column histograms and a sliding window kernel histogram
     * to achieve O(N) complexity.
     *
     * @param input Input image view.
     * @param radius Radius of the filter.
     */
    void applyPerreault(const ImageView& input, int radius);

    /**
     * @brief Apply naive median filter.
     *
     * Uses direct neighborhood extraction and partial sorting.
     *
     * @param input Input image view.
     * @param radius Radius of the filter.
     */
    void applyNaive(const ImageView& input, int radius);

    static constexpr int kHistogramSize{256}; ///< Histogram size for 8-bit images.

    /**
     * @brief Reset histogram values to zero.
     */
    template <typename Container>
    void clearHistogram(Container& histo)
    {
        std::fill(histo.begin(), histo.end(), 0);
    }

    /**
     * @brief Initialize current median index.
     *
     * Starts from the middle of the histogram.
     */
    void initCurrentMedian()
    {
        currentMedian_ = kHistogramSize / 2;
    }

    /**
     * @brief Clamp x-coordinate to valid image range.
     */
    int clampX(int x) const noexcept
    {
        return std::clamp(x, 0, width_ - 1);
    }

    /**
     * @brief Accumulate a column histogram into the kernel histogram.
     */
    void accumulateColumn(int colIndex);

    /**
     * @brief Remove a column histogram from the kernel histogram.
     */
    void removeColumn(int colIndex);

    /**
     * @brief Update kernel histogram by adding and removing columns.
     */
    void updateKernel(int addColIndex, int removeColIndex);

    /**
     * @brief Find median value from the histogram.
     *
     * @param targetRank Rank of the median in the histogram.
     * @return Median value.
     */
    uint8_t findMedian(int targetRank);

    /**
     * @brief Initialize column histograms.
     */
    void initColumnsHisto(const ImageView& input, int ch, int kernelSize);

    /**
     * @brief Update a column histogram incrementally.
     */
    void updateColumnsHisto(int colIndex, uint8_t valRemove, uint8_t valAdd);

    ImageOwner buffer_; ///< Intermediate buffer (used for processing).
    ImageOwner output_; ///< Output buffer.

    std::vector<int> columnsHisto_;                 ///< Column histograms (size = width * 256).
    std::array<int, kHistogramSize> kernelHisto_{}; ///< Kernel histogram.

    int currentMedian_{kHistogramSize / 2}; ///< Current median index.

    int width_{0};    ///< Cached image width.
    int height_{0};   ///< Cached image height.
    int channels_{0}; ///< Number of channels.
};

} // namespace fluvel_ip::filter
