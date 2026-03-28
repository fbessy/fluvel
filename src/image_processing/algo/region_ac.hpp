// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

class RegionAc : public ActiveContour
{
public:
    //! Constructor to initialize with an initial contour.
    RegionAc(ImageView image, ContourData initialContour,
             const AcConfig& generalConfig = AcConfig(),         /* optional parameter */
             const RegionConfig& regionConfig = RegionConfig()); /* optional parameter */

    ~RegionAc() override = default;

    //! Getter function for #Cout.
    int get_Cout() const
    {
        return meanOut_;
    }
    //! Getter function for #Cin.
    int get_Cin() const
    {
        return meanIn_;
    }

    void fillDiagnostics(ContourDiagnostics& d) const override;

private:
    //! Initializes the variables #sum_in, #sum_out and #pxl_nbr_out with scanning through the
    //! image.
    void initialize_sums();

    //! Calculates means #Cout and #Cin in \a O(1) or accounting for the previous updates of
    //! #sum_out and #sum_in, in \a O(#lists_length) and not in \a O(#img_size).
    void onStepCycle1() override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of
    //! #l_out or #l_in.
    void computeSpeed(ContourPoint& point) override;

    //! Updates variables #sum_in, #sum_out and #pxl_nbr_out in order to calculate the means
    //! #average_out and #average_in in constant time ( complexity 0(1) ).
    void onSwitch(const ContourPoint& point, BoundarySwitch ctxChoice) override;

    //! Image wrapper.
    ImageView image_;

    //! Specific configuration for region based active contour.
    const RegionConfig regionConfig_;

    //! Average or mean of the intensities or grey-levels of the pixels outside the curve,
    //! called C2 in the Chan-Vese article.
    int meanOut_;

    //! Average or mean of the intensities or grey-levels of the pixels inside the curve, called
    //! C1 in the Chan-Vese article.
    int meanIn_;

    //! Sum of the intensities or grey-levels of the whole image's pixels.
    int64_t sum_total_{0};
    //! Number of pixels or bytes of #img_data.
    const int64_t pxl_nbr_total_;

    //! Sum of the intensities or grey-levels of the pixels outside the curve, i.e. pixels
    //! \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int64_t sum_out_{0};
    //! Number of pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! >0\f$ .
    int64_t pxl_nbr_out_{0};
};

} // namespace fluvel_ip
