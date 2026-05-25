// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "image_view.hpp"
#include "speed_model.hpp"
#include "speed_model_types.hpp"

#include <cstdint>

namespace fluvel_ip
{

/**
 * @brief Region-based color speed model for active contours (Chan-Vese).
 *
 * This model computes the external speed Fd based on region statistics
 * (mean color inside and outside the contour).
 *
 * The contour evolves to minimize the difference between pixel values
 * and the corresponding region mean (inside vs outside).
 *
 * The implementation maintains incremental statistics to achieve
 * efficient updates in O(1) per iteration instead of O(N).
 */
class RegionColorSpeedModel : public ISpeedModel
{
public:
    /**
     * @brief Constructor.
     *
     * @param params Configuration parameters for the region-based model.
     */
    RegionColorSpeedModel(const RegionColorParams& params = RegionColorParams());

    /// Destructor.
    ~RegionColorSpeedModel() override = default;

    /**
     * @brief Initialize internal statistics from the input image and contour.
     *
     * Computes initial sums and pixel counts for inside and outside regions.
     *
     * @param image Input image.
     * @param contour Initial contour data.
     */
    void onImageChanged(const ImageView& image, const ContourData& contour) override;

    /**
     * @brief Update region means at the beginning of Cycle 1.
     *
     * Means are computed incrementally from maintained sums,
     * avoiding a full image scan.
     */
    void onStepCycle1() override;

    /**
     * @brief Update region statistics when a point switches region.
     *
     * This updates sums and counters incrementally during contour evolution.
     *
     * @param point Contour point being switched.
     * @param direction Direction of the switch (in/out).
     */
    void onSwitch(const ContourPoint& point, SwitchDirection direction) override;

    /**
     * @brief Compute external speed Fd for a contour point.
     *
     * The speed is based on the Chan-Vese energy formulation,
     * comparing pixel value to region means.
     *
     * @param point Contour point to update.
     * @param DiscreteLevelSet Unused here but required by interface.
     */
    void computeSpeed(ContourPoint& point, const DiscreteLevelSet&) override;

    /**
     * @brief Fill diagnostic information.
     *
     * @param d Output diagnostics structure.
     */
    void fillDiagnostics(ContourDiagnostics& d) const override;

    /**
     * @brief Get mean RGB color outside the contour.
     */
    const Rgb_uc& meanOutside() const
    {
        return meanRgbOutside_;
    }

    /**
     * @brief Get mean RGB color inside the contour.
     */
    const Rgb_uc& meanInside() const
    {
        return meanRgbInside_;
    }

private:
    /**
     * @brief Convert RGB color to internal color space.
     *
     * Depending on configuration, this may convert to YUV or keep RGB.
     *
     * @param rgb Input RGB color.
     * @return Converted 3-channel color.
     */
    inline Color_3i rgbToColor(const Rgb_uc& rgb) const;

    /// Reference to the current image.
    ImageView image_;

    /// Configuration parameters for the region-based model.
    const RegionColorParams params_;

    /// Mean color (internal color space) outside the contour.
    Color_3i meanColorOut_{0, 0, 0};

    /// Mean color (internal color space) inside the contour.
    Color_3i meanColorIn_{0, 0, 0};

    /// Mean RGB color outside the contour.
    Rgb_uc meanRgbOutside_{0u, 0u, 0u};

    /// Mean RGB color inside the contour.
    Rgb_uc meanRgbInside_{0u, 0u, 0u};

    /// Total sum of RGB components over the whole image.
    Rgb_64i sumTotal_{0, 0, 0};

    /// Total number of pixels in the image.
    int64_t pixelCountTotal_{0};

    /// Sum of RGB components outside the contour.
    Rgb_64i sumOutside_{0, 0, 0};

    /// Number of pixels outside the contour.
    int64_t pixelCountOutside_{0};

    /// Indicates whether initial means have been validated.
    bool initialMeansChecked_{false};
};

} // namespace fluvel_ip
