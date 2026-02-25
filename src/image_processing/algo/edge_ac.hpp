// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "image_span.hpp"

namespace ofeli_ip
{

class EdgeAc : public ActiveContour
{
public:
    //! Constructor to initialize with an initial contour.
    template <typename T>
    EdgeAc(ImageSpan gradient_image, T&& initial_contour,
           const AcConfig& config = AcConfig()); /* optional parameter */

    //! Getter function for #threshold
    unsigned char get_threshold() const
    {
        return threshold_;
    }

private:
    //! Computes external speed \a Fd with a geodesic model for a current point \a (x,y) of
    //! #l_out or #l_in.
    void compute_external_speed_Fd(ContourPoint& point) override;

    //! Gets the global speed sign to evolve the initial active contour is the good way.
    int get_global_speed_sign() const;

    //! Otsu's method to calculate an optimal global threshold.
    static unsigned char do_otsu_method(ImageSpan image);

    //! Image wrapper.
    ImageSpan gradient_image_;

    //! Global speed sign (1 or -1).
    const int global_speed_sign_;

    //! Global optimal threshold of #img_gradient_data.
    const unsigned char threshold_;
};

template <typename T>
EdgeAc::EdgeAc(ImageSpan gradient_image, T&& initial_contour,
               const AcConfig& config) /* optional parameter with AcConfig() */
    : ActiveContour(std::forward<T>(initial_contour), config)
    , gradient_image_(gradient_image)
    , global_speed_sign_(get_global_speed_sign())
    , threshold_(do_otsu_method(gradient_image))
{
    assert(gradient_image_.width() == cd_.phi().width() &&
           gradient_image_.height() == cd_.phi().height());
}

} // namespace ofeli_ip
