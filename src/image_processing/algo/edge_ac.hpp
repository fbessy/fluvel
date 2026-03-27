// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

class EdgeAc : public ActiveContour
{
public:
    //! Constructor to initialize with an initial contour.
    EdgeAc(ImageView gradient_image, ContourData initialContour,
           const AcConfig& config = AcConfig()); /* optional parameter */

    ~EdgeAc() override = default;

    //! Getter function for #threshold
    unsigned char get_threshold() const
    {
        return threshold_;
    }

private:
    //! Computes external speed \a Fd with a geodesic model for a current point \a (x,y) of
    //! #l_out or #l_in.
    void computeExternalSpeedFd(ContourPoint& point) override;

    //! Gets the global speed sign to evolve the initial active contour is the good way.
    int get_global_speed_sign() const;

    //! Otsu's method to calculate an optimal global threshold.
    static unsigned char do_otsu_method(ImageView image);

    //! Image wrapper.
    ImageView gradient_image_;

    //! Global speed sign (1 or -1).
    const int global_speed_sign_;

    //! Global optimal threshold of #img_gradient_data.
    const unsigned char threshold_;
};

} // namespace fluvel_ip
