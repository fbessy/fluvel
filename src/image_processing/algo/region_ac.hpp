/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef REGION_AC_HPP
#define REGION_AC_HPP

#include "active_contour.hpp"
#include "image_span.hpp"

namespace ofeli_ip
{

//! \class RegionConfig
//! Specific configuration for region based active contour
struct RegionConfig
{
    static constexpr int kDefaultLambdaIn = 1;
    static constexpr int kDefaultLambdaOut = 1;

    //! Weight of the inside homogeneity criterion in the Chan-Vese model
    //! (called lambda 1 in the article "Active contour without edges.").
    int lambda_in;

    //! Weight of the outside homogeneity criterion in the Chan-Vese model
    //! (called lambda 2 in the article "Active contour without edges.").
    int lambda_out;

    //! Check values of a configuration.
    void normalize()
    {
        lambda_in = normalize(lambda_in);
        lambda_out = normalize(lambda_out);
    }

    //! Default constructor.
    RegionConfig()
        : lambda_in(kDefaultLambdaIn)
        , lambda_out(kDefaultLambdaOut)
    {
    }

    //! Destructor.
    virtual ~RegionConfig()
    {
    }

    //! Copy constructor.
    RegionConfig(const RegionConfig& copied)
        : lambda_in(copied.lambda_in)
        , lambda_out(copied.lambda_out)
    {
        this->normalize();
    }

    //! Copy assignement operator.
    RegionConfig& operator=(const RegionConfig& rhs)
    {
        this->lambda_in = rhs.lambda_in;
        this->lambda_out = rhs.lambda_out;

        this->normalize();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const RegionConfig& lhs, const RegionConfig& rhs)
    {
        return (lhs.lambda_in == rhs.lambda_in && lhs.lambda_out == rhs.lambda_out);
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const RegionConfig& lhs, const RegionConfig& rhs)
    {
        return !(lhs == rhs);
    }

protected:
    //! Normalize value weight and returns the same value or a default value.
    static int normalize(int weight)
    {
        if (weight < 1)
            weight = 1;

        return weight;
    }
};

class RegionAc : public ActiveContour
{
public:
    //! Constructor to initialize with an initial contour.
    template <typename T>
    RegionAc(ImageSpan image, T&& initial_contour,
             const AcConfig& general_config = AcConfig(),         /* optional parameter */
             const RegionConfig& region_config = RegionConfig()); /* optional parameter */

    //! Getter function for #Cout.
    int get_Cout() const
    {
        return average_out_;
    }
    //! Getter function for #Cin.
    int get_Cin() const
    {
        return average_in_;
    }

private:
    //! Initializes the variables #sum_in, #sum_out and #pxl_nbr_out with scanning through the
    //! image.
    void initialize_sums();

    //! Calculates means #Cout and #Cin in \a O(1) or accounting for the previous updates of
    //! #sum_out and #sum_in, in \a O(#lists_length) and not in \a O(#img_size).
    void do_specific_cycle1() override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of
    //! #l_out or #l_in.
    void compute_external_speed_Fd(ContourPoint& point) override;

    //! Updates variables #sum_in, #sum_out and #pxl_nbr_out in order to calculate the means
    //! #average_out and #average_in in constant time ( complexity 0(1) ).
    void do_specific_when_switch(const ContourPoint& point, BoundarySwitch ctx_choice) override;

    //! Image wrapper.
    ImageSpan image_;

    //! Specific configuration for region based active contour.
    const RegionConfig region_config_;

    //! Average or mean of the intensities or grey-levels of the pixels outside the curve,
    //! called C2 in the Chan-Vese article.
    int average_out_;

    //! Average or mean of the intensities or grey-levels of the pixels inside the curve, called
    //! C1 in the Chan-Vese article.
    int average_in_;

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

template <typename T>
RegionAc::RegionAc(ImageSpan image, T&& initial_contour,
                   const AcConfig& general_config,    /* optional parameter with AcConfig() */
                   const RegionConfig& region_config) /* optional parameter with RegionConfig() */
    : ActiveContour(std::forward<T>(initial_contour), general_config)
    , image_(image)
    , region_config_(region_config)
    , pxl_nbr_total_(image.size())
{
    assert(image.width() == cd_.phi().width() && image.height() == cd_.phi().height());

    initialize_sums();
    RegionAc::do_specific_cycle1();
}

} // namespace ofeli_ip

#endif // REGION_AC_HPP

//! \class ofeli::RegionAc
//! The child class RegionAc implements a function to calculate specifically speed \a Fd based on
//! the Chan-Vese model, a region-based energy functional. The regularization of our active contour
//! is performed by a gaussian smoothing of #phi so we are interested uniquely by the external or
//! data dependant term of this energy functional.\n\n
//! \f$F_{d}=\lambda _{out}\left( I-C_{out}\right) ^{2}- \lambda _{in}\left( I-C_{in}\right) ^{2}\f$
//! \n\n
//!  - \f$F_{d}\f$ : data dependant evolution speed calculated for each point of the active contour,
//!  only it sign is used by the algorithm. \n
//!  - \f$I\f$ : intensity or grey-level of the current pixel of the active contour. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e.
//!  pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$\lambda _{out}\f$ : weight of the outside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\lambda _{in}\f$ : weight of the inside homogeneity criterion in the Chan-Vese model.

/**
 * \fn RegionAc::RegionAc(const unsigned char* img_data1, int img_width1, int img_height1,
                                      bool has_ellipse1, double shape_width_ratio1, double
 shape_height_ratio1, double center_x_ratio1, double center_y_ratio1, bool has_cycle2_1, int
 kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1, int lambda_out1, int lambda_in1)
 * \param img_data1 Input pointer on the grayscale or grey-level image data buffer. This buffer must
 be row-wise. Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param has_ellipse1 Boolean to choose the shape of the active contour initialization, \c true for
 an ellipse or \c false for a rectangle.
 * \param shape_width_ratio1 Width of the shape divided by the image #img_width.
 * \param shape_height_ratio1 Height of the shape divided by the image #img_height.
 * \param center_x_ratio1 X-axis position (or column index) of the center of the shape divided by
 the image #img_width subtracted by 0.5.
 * \param center_y_ratio1 Y-axis position (or row index) of the center of the shape divided by the
 image #img_height subtracted by 0.5.\n To have the center of the shape in the image : -0.5 <
 center_x_ratio1 < 0.5 and -0.5 < center_y_ratio1 < 0.5.
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an
 internal speed \a Fint. Passed to #is_cycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant
 evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal
 evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambda_out.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambda_in.
 */

/**
 * \fn RegionAc::RegionAc(const unsigned char* img_data1, int img_width1, int img_height1,
 *                                                        const char* phi_init1,
 *                                                        bool has_cycle2_1, int kernel_length1,
 * double sigma1, unsigned int Na1, unsigned int Ns1, int lambda_out1, int lambda_in1)
 * \param img_data1 Input pointer on the grayscale image data buffer. This buffer must be row-wise.
 * Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi.
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an
 * internal speed \a Fint. Passed to #is_cycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant
 * evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal
 * evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambda_out.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambda_in.
 */

/**
 * \fn virtual signed char RegionAc::compute_external_speed_Fd(ContourPoint& point)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual void RegionAc::updates_for_means_in2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual void RegionAc::updates_for_means_out2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */
