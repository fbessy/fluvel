// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"
#include "speed_model.hpp"
#include "speed_model_types.hpp"

namespace fluvel_ip
{

/**
 * @brief Region-based grayscale speed model for active contours (Chan-Vese).
 *
 * This model computes the external speed Fd using intensity statistics
 * (mean gray level inside and outside the contour).
 *
 * It is a simplified version of the Chan-Vese model for single-channel images.
 *
 * The implementation maintains incremental sums to allow efficient updates
 * in O(1) per iteration instead of rescanning the whole image.
 */
class RegionGraySpeedModel : public ISpeedModel
{
public:
    /**
     * @brief Constructor.
     *
     * @param params Configuration parameters for the region-based model.
     */
    RegionGraySpeedModel(const RegionParams& params = RegionParams());

    /// Destructor.
    ~RegionGraySpeedModel() override = default;

    /**
     * @brief Initialize internal statistics from the input image and contour.
     *
     * Computes initial sums and pixel counts for inside and outside regions.
     *
     * @param image Input grayscale image.
     * @param cd Contour data.
     */
    void onImageChanged(const ImageView& image, const ContourData& cd) override;

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
     * The speed is computed using the Chan-Vese energy model
     * based on the difference between pixel intensity and region means.
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
     * @brief Get mean intensity outside the contour.
     */
    int meanOutside() const
    {
        return meanOutside_;
    }

    /**
     * @brief Get mean intensity inside the contour.
     */
    int meanInside() const
    {
        return meanInside_;
    }

private:
    /// Reference to the current image.
    ImageView image_;

    /// Configuration parameters for the region-based model.
    const RegionParams params_;

    /// Mean intensity outside the contour (C_out in Chan-Vese).
    int meanOutside_{0};

    /// Mean intensity inside the contour (C_in in Chan-Vese).
    int meanInside_{0};

    /// Sum of intensities over the whole image.
    int64_t sumTotal_{0};

    /// Total number of pixels in the image.
    int64_t pixelCountTotal_{0};

    /// Sum of intensities outside the contour.
    int64_t sumOutside_{0};

    /// Number of pixels outside the contour.
    int64_t pixelCountOutside_{0};

    /// Indicates whether initial means have been validated.
    bool initialMeansChecked_{false};
};

} // namespace fluvel_ip
