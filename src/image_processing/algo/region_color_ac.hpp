// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "color.hpp"
#include "image_view.hpp"
#include "speed_model.hpp"

#include <cstdint>

namespace fluvel_ip
{

class RegionColorSpeedModel : public ISpeedModel
{
public:
    ///! Constructor to initialize with an initial contour.
    RegionColorSpeedModel(const RegionColorParams& configuration =
                              RegionColorParams()); /* optional parameter */

    ~RegionColorSpeedModel() override = default;

    //! Initializes the six sums and #n_in and #n_out with scanning through the image.
    void onImageChanged(ImageView image, const ContourData& contour) override;

    //! Calculates means #CoutYUV and #CinYUV in \a O(1) or accounting for the previous updates
    //! of (#sum_out_R, #sum_out_G, #sum_out_B) and (#sum_in_R, #sum_in_G, #sum_in_B), in \a
    //! O(#lists_length) and not in \a O(#img_size).
    void onStepCycle1() override;

    //! Updates the six sums, #n_in and #n_out, before each #switch_in, in the cycle 1, in order
    //! to calculate means #CoutYUV and #CinYUV.
    void onSwitch(const ContourPoint& point, SwitchDirection direction) override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of
    //! #outerBoundary or #innerBoundary.
    void computeSpeed(ContourPoint& point, const DiscreteLevelSet&) override;

    void fillDiagnostics(ContourDiagnostics& d) const override;

    //! Getter function for #average_rgb_out
    const Rgb_uc& meanOut() const
    {
        return meanRgbOut_;
    }

    //! Getter function for #average_rgb_in
    const Rgb_uc& meanIn() const
    {
        return meanRgbIn_;
    }

private:

    //! Calculates components \a of a color space (in function of the color space option) with a
    //! rgb value.
    inline Color_3i rgbToColor(const Rgb_uc& rgb) const;

    //! Image wrapper.
    ImageView image_;

    //! Specific configuration for YUV region based active contour.
    const RegionColorParams params_;

    //! Mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! >0\f$ .
    Color_3i meanColorOut_{0, 0, 0};

    //! Mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! <0\f$ .
    Color_3i meanColorIn_{0, 0, 0};

    //! RGB mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left(
    //! i\right) >0\f$ .
    Rgb_uc meanRgbOut_{0u, 0u, 0u};

    //! RGB mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left(
    //! i\right) <0\f$ .
    Rgb_uc meanRgbIn_{0u, 0u, 0u};

    //! Sum of component #R, #G, #B of the pixels intside the curve, i.e. pixels \f$i\f$ with
    //! \f$\phi \left( i\right) <0\f$ .
    Rgb_64i sumTotal_{0, 0, 0};
    //! Number of pixels or bytes of #phi.
    int64_t pxlNbrTotal_;

    //! Sum of component #R, #G, #B of the pixels outside the curve, i.e. pixels \f$i\f$ with
    //! \f$\phi \left( i\right) >0\f$ .
    Rgb_64i sumOut_{0, 0, 0};
    //! Number of pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! >0\f$ .
    int64_t pxlNbrOut_{0};

    bool initialMeansChecked_{false};
};

} // namespace fluvel_ip
