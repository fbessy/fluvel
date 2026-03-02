// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "color.hpp"
#include "image_span.hpp"

#include <cstdint>

namespace ofeli_ip
{

class RegionColorAc : public ActiveContour
{
public:
    ///! Constructor to initialize with an initial contour.
    template <typename T>
    RegionColorAc(
        ImageSpan image, T&& initial_contour,
        const AcConfig& general_config = AcConfig(),                   /* optional parameter */
        const RegionColorConfig& regionConfig = RegionColorConfig()); /* optional parameter */

    ~RegionColorAc() override = default;

    //! Reset the execution state with a new image buffer. Used for video tracking.
    void resetExecutionState(ImageSpan image);

    //! Getter function for #average_rgb_out
    const Rgb_uc& get_Cout() const
    {
        return meanOut_;
    }

    //! Getter function for #average_rgb_in
    const Rgb_uc& get_Cin() const
    {
        return meanIn_;
    }

    void fillDiagnostics(ContourDiagnostics& d) const override;

private:
    //! Initializes the six sums and #n_in and #n_out with scanning through the image.
    void initialize_sums();

    //! Calculates means #CoutYUV and #CinYUV in \a O(1) or accounting for the previous updates
    //! of (#sum_out_R, #sum_out_G, #sum_out_B) and (#sum_in_R, #sum_in_G, #sum_in_B), in \a
    //! O(#lists_length) and not in \a O(#img_size).
    void do_specific_cycle1() override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of
    //! #l_out or #l_in.
    void computeExternalSpeedFd(ContourPoint& point) override;

    //! Updates the six sums, #n_in and #n_out, before each #switch_in, in the cycle 1, in order
    //! to calculate means #CoutYUV and #CinYUV.
    void doSpecificWhenSwitch(const ContourPoint& point, BoundarySwitch ctxChoice) override;

    //! Calculates components \a of a color space (in function of the color space option) with a
    //! rgb value.
    inline Color_3i rgb_to_color(const Rgb_uc& rgb) const;

    //! Image wrapper.
    ImageSpan image_;

    //! Specific configuration for YUV region based active contour.
    const RegionColorConfig regionConfig_;

    //! Mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! >0\f$ .
    Color_3i average_color_out_;

    //! Mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! <0\f$ .
    Color_3i average_color_in_;

    //! RGB mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left(
    //! i\right) >0\f$ .
    Rgb_uc meanOut_;

    //! RGB mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left(
    //! i\right) <0\f$ .
    Rgb_uc meanIn_;

    //! Sum of component #R, #G, #B of the pixels intside the curve, i.e. pixels \f$i\f$ with
    //! \f$\phi \left( i\right) <0\f$ .
    Rgb_64i sum_total_;
    //! Number of pixels or bytes of #phi.
    const int64_t pxl_nbr_total_;

    //! Sum of component #R, #G, #B of the pixels outside the curve, i.e. pixels \f$i\f$ with
    //! \f$\phi \left( i\right) >0\f$ .
    Rgb_64i sum_out_;
    //! Number of pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right)
    //! >0\f$ .
    int64_t pxl_nbr_out_{0};

    Rgb_64i sum2_out_;
    Rgb_64i sum2_total_;
};

// Definitions

template <typename T>
RegionColorAc::RegionColorAc(
    ImageSpan image, T&& initial_contour,
    const AcConfig& general_config,         /* optional parameter with AcConfig() */
    const RegionColorConfig& regionConfig) /* optional parameter with RegionColorConfig() */
    : ActiveContour(std::forward<T>(initial_contour), general_config)
    , image_(image)
    , regionConfig_(regionConfig)
    , pxl_nbr_total_(image.size())
{
    assert(image.width() == cd_.phi().width() && image.height() == cd_.phi().height());

    initialize_sums();
    RegionColorAc::do_specific_cycle1();
}

} // namespace ofeli_ip

//! \class ofeli::RegionColorAc
//! The child class RegionColorAc implements a function to calculate specifically speed \a Fd based
//! on the Chan-Vese model, a region-based energy functional. The regularization of our active
//! contour is performed by a gaussian smoothing of #phi so we are interested uniquely by the
//! external or data dependant term of this energy functional.\n
//! \f$F_{d}=\lambda _{out}\left[ \alpha \left( Y_{out}-C_{outC}\right) ^{2}+ \beta \left(
//! U_{out}-C_{outU}\right) ^{2}+ \gamma \left( V_{out}-C_{outV}\right) ^{2}\right] + \lambda
//! _{in}\left[ \alpha \left( Y_{in}-C_{inY}\right) ^{2}+ \beta \left( U_{in}-C_{inU}\right) ^{2}+
//! \gamma \left( V_{in}-C_{inV}\right) ^{2}\right]\f$
//!  - \f$F_{d}\f$ : data dependant evolution speed calculated for each point of the active contour,
//!  only it sign is used by the algorithm. \n
//!  - \f$Y\f$ : luminance component Y of the (Y,U,V) color space of the current pixel of the active
//!  contour. \n
//!  - \f$U\f$ : chrominance component U of the (Y,U,V) color space of the current pixel of the
//!  active contour. \n
//!  - \f$V\f$ : chrominance component V of the (Y,U,V) color space of the current pixel of the
//!  active contour. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$\lambda _{out}\f$ : weight of the outside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\lambda _{in}\f$ : weight of the inside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\alpha\f$ : weight of the luminance component Y. \n
//!  - \f$\beta\f$ : weight of the chrominance component U. \n
//!  - \f$\gamma\f$ : weight of the chrominance component V.

/**
 * \fn RegionColorAc::RegionColorAc(const unsigned char* img_rgb_data1, int img_width1, int
 img_height1, bool has_ellipse1, double shape_width_ratio1, double shape_height_ratio1, double
 center_x_ratio1, double center_y_ratio1, bool has_cycle2_1, int kernel_length1, double sigma1,
 unsigned int Na1, unsigned int Ns1, int lambda_out1, int lambda_in1, int alpha1, int beta1, int
 gamma1)
 * \param img_rgb_data1 Input pointer on the RGB image data buffer. This buffer must be row-wise and
 interleaved (R1 G1 B1 R2 G2 B2 ...). Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param has_ellipse1 Boolean to choose the shape of the active contour initialization, \c true for
 an ellipse or \c false for a rectangle.
 * \param shape_width_ratio1 Width of the shape divided by the image #img_width.
 * \param shape_height_ratio1 Height of the shape divided by the image #img_height.
 * \param center_x_ratio1 X-axis position (or column index) of the center of the shape divided by
 the image #img_width subtracted by 0.5
 * \param center_y_ratio1 Y-axis position (or row index) of the center of the shape divided by the
 image #img_height subtracted by 0.5\n To have the center of the shape in the image : -0.5 <
 center_x_ratio1 < 0.5 and -0.5 < center_y_ratio1 < 0.5
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an
 internal speed \a Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to
 #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to
 #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant
 evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal
 evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambdaOut.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambdaIn.
 * \param alpha1 Weight of luminance Y. Passed to #alpha.
 * \param beta1 Weight of chrominance U. Passed to #beta.
 * \param gamma1 Weight of chrominance V. Passed to #gamma.
 */

/**
 * \fn RegionColorAc::RegionColorAc(const unsigned char* img_rgb_data1, int img_width1, int
 img_height1, const char* phi_init1, bool has_cycle2_1, int kernel_length1, double sigma1, unsigned
 int Na1, unsigned int Ns1, int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1)
 * \param img_rgb_data1 Input pointer on the RGB data image buffer. This buffer must be row-wise and
 interleaved (R1 G1 B1 R2 G2 B2 ...). Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi.
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an
 internal speed \a Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to
 #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to
 #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant
 evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal
 evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambdaOut.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambdaIn.
 * \param alpha1 Weight of luminance Y. Passed to #alpha.
 * \param beta1 Weight of chrominance U. Passed to #beta.
 * \param gamma1 Weight of chrominance V. Passed to #gamma.
 */

/**
 * \fn virtual signed char RegionColorAc::computeExternalSpeedFd(ContourPoint& point)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual void RegionColorAc::updates_for_means_in2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual RegionColorAc::updates_for_means_out2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */
