// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "image_view.hpp"
#include "speed_model.hpp"

namespace fluvel_ip
{

class RegionSpeedModel : public ISpeedModel
{
public:
    //! Constructor to initialize with an initial contour.
    RegionSpeedModel(const RegionParams& configuration = RegionParams()); /* optional parameter */

    ~RegionSpeedModel() override = default;

    //! Initializes the variables #sumIn, #sum_out and #pxl_nbr_out with scanning through the
    //! image.
    void onImageChanged(ImageView image, const ContourData& cd) override;

    //! Calculates means #Cout and #Cin in \a O(1) or accounting for the previous updates of
    //! #sum_out and #sumIn, in \a O(#lists_length) and not in \a O(#img_size).
    void onStepCycle1() override;

    //! Updates variables #sumIn, #sum_out and #pxl_nbr_out in order to calculate the means
    //! #average_out and #average_in in constant time ( complexity 0(1) ).
    void onSwitch(const ContourPoint& point, SwitchDirection direction) override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of
    //! #l_out or #l_in.
    void computeSpeed(ContourPoint& point, const DiscreteLevelSet&) override;

    void fillDiagnostics(ContourDiagnostics& d) const override;

    //! Getter function for #Cout.
    int meanOut() const
    {
        return meanOut_;
    }
    //! Getter function for #Cin.
    int meanIn() const
    {
        return meanIn_;
    }

private:

    //! Image wrapper.
    ImageView image_;

    //! Specific configuration for region based active contour.
    const RegionParams params_;

    //! Average or mean of the intensities or grey-levels of the pixels outside the curve,
    //! called C2 in the Chan-Vese article.
    int meanOut_{0};

    //! Average or mean of the intensities or grey-levels of the pixels inside the curve, called
    //! C1 in the Chan-Vese article.
    int meanIn_{0};

    //! Sum of the intensities or grey-levels of the whole image's pixels.
    int64_t sumTotal_{0};
    //! Number of pixels or bytes of #img_data.
    int64_t pxlNbrTotal_{0};

    //! Sum of the intensities or grey-levels of the pixels outside the curve, i.e. pixels
    //! \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int64_t sumOut_{0};
    //! Number of pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! >0\f$ .
    int64_t pxlNbrOut_{0};
};

} // namespace fluvel_ip
