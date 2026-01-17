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

#ifndef REGION_COLOR_AC_HPP
#define REGION_COLOR_AC_HPP

#include "active_contour.hpp"
#include "region_ac.hpp"
#include "image_span.hpp"

namespace ofeli_ip
{

constexpr int CHANNELS_NBR = 3;

/**
 * @brief Inverse sRGB gamma correction, transforms R' to R
 */
constexpr float INV_GAMMA_CORRECTION(float val)
{
    return val <= 0.0404482362771076f ?
               val/12.92f : std::pow(((val) + 0.055f)/1.055f, 2.4f);
}

/** @brief XYZ color of the D65 white point */
constexpr auto WHITE_POINT_X = 0.950456f;
constexpr auto WHITE_POINT_Y = 1.f;
constexpr auto WHITE_POINT_Z = 1.088754f;

/** @brief *u*v color of the D65 white point */
constexpr auto WHITE_POINT_DENOM = WHITE_POINT_X + 15.f*WHITE_POINT_Y + 3.f*WHITE_POINT_Z;
constexpr auto WHITE_POINT_U     = 4.f*WHITE_POINT_X / WHITE_POINT_DENOM;
constexpr auto WHITE_POINT_V     = 9.f*WHITE_POINT_Y / WHITE_POINT_DENOM;

/**
 * @brief CIE L*a*b* f function (used to convert XYZ to L*a*b*)
 * http://en.wikipedia.org/wiki/Lab_color_space
 */
constexpr float LAB_FUNC(float val)
{
    return val >= float(8.85645167903563082e-3) ?
               std::cbrtf(val) : (841.f/108.f)*(val) + (4.f/29.f);
}

enum ColorSpaceOption : unsigned int
{
    RGB          = 0,
    YUV          = 1,
    Lab          = 2,
    Luv          = 3
};

//! \class RegionColorConfig
//! Specific configuration for color region based active contour.
struct RegionColorConfig : public RegionConfig
{
    //! Color space option
    ColorSpaceOption color_space;

    //! Weights \a to calculate external speed \a Fd.
    int weights[CHANNELS_NBR];

    //! Check values of a configuration.
    void check_region_color()
    {
        for( int chl_idx = 0; chl_idx < CHANNELS_NBR; chl_idx++ )
        {
            weights[chl_idx] = check( weights[chl_idx] );
        }
    }

    //! Default constructor.
    RegionColorConfig() : RegionConfig(),
        color_space(ColorSpaceOption::RGB)
    {
        for( int chl_idx = 0; chl_idx < CHANNELS_NBR; chl_idx++ )
        {
            weights[chl_idx] = 1;
        }
    }

    //! Copy constructor.
    RegionColorConfig(const RegionColorConfig& copied) : RegionConfig(copied),
        color_space(copied.color_space)
    {
        for( int chl_idx = 0; chl_idx < CHANNELS_NBR; chl_idx++ )
        {
            weights[chl_idx] = copied.weights[chl_idx];
        }

        this->check_region_color();
    }

    //! Copy assignement operator.
    RegionColorConfig& operator=(const RegionColorConfig& rhs)
    {
        RegionConfig::operator=(rhs);

        this->color_space = rhs.color_space;

        for( int chl_idx = 0; chl_idx < CHANNELS_NBR; chl_idx++ )
        {
            this->weights[chl_idx] = rhs.weights[chl_idx];
        }

        this->check_region_color();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const RegionColorConfig& lhs,
                           const RegionColorConfig& rhs)
    {
        bool is_same_weight = true;

        for( int chl_idx = 0;
             chl_idx < CHANNELS_NBR && is_same_weight;
             chl_idx++ )
        {
            if( lhs.weights[chl_idx] != rhs.weights[chl_idx] )
            {
                is_same_weight = false;
            }
        }

        return (    lhs.color_space == rhs.color_space
                 && lhs.lambda_in   == rhs.lambda_in
                 && lhs.lambda_out  == rhs.lambda_out
                 && is_same_weight );
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const RegionColorConfig& lhs,
                           const RegionColorConfig& rhs)
    {
        return !(lhs == rhs);
    }
};

class RegionColorAc : public ActiveContour
{

public :

    ///! Constructor to initialize with an initial contour.
    template<typename T>
    RegionColorAc(ImageSpan32 image1,
                 T&& initial_contour1,
                 const AcConfig& general_config1 = AcConfig(),                       /* optional parameter */
                 const RegionColorConfig& region_config1 = RegionColorConfig()); /* optional parameter */

    //! Reinitializes the active contour with a new image buffer. Used for video tracking.
    void reinitialize(ImageSpan32 image1);

    //! Getter function for #average_rgb_out
    const Rgb_uc& get_Cout() const { return average_out; }

    //! Getter function for #average_rgb_in
    const Rgb_uc& get_Cin() const { return average_in; }

private :

    //! Initializes the six sums and #n_in and #n_out with scanning through the image.
    void initialize_sums();

    //! Calculates means #CoutYUV and #CinYUV in \a O(1) or accounting for the previous updates of (#sum_out_R, #sum_out_G, #sum_out_B) and (#sum_in_R, #sum_in_G, #sum_in_B), in \a O(#lists_length) and not in \a O(#img_size).
    virtual void do_specific_cycle1() override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of #l_out or #l_in.
    virtual void compute_external_speed_Fd(ContourPoint& point) override;

    //! Updates the six sums, #n_in and #n_out, before each #switch_in, in the cycle 1, in order to calculate means #CoutYUV and #CinYUV.
    virtual void do_specific_when_switch(int offset,
                                         BoundarySwitch ctx_choice) override;


    //! Color space conversion functions.
    static void rgb_to_xyz(float R, float G, float B,
                           float *X, float *Y, float *Z);

    static void xyz_to_Lab(float X, float Y, float Z,
                           float *L, float *a, float *b);

    static void xyz_to_Luv(float X, float Y, float Z,
                           float *L, float *u, float *v);

    static void rgb_to_Lab(float R, float G, float B,
                           float *L, float *a, float *b);

    static void rgb_to_Luv(float R, float G, float B,
                           float *L, float *u, float *v);



    //! Calculates components \a of a color space (in function of the color space option) with a rgb value.
    void rgb_to_color(const Rgb_uc& rgb,
                      int color[]) const;

    //! Image wrapper.
    ImageSpan32 image;

    //! Specific configuration for YUV region based active contour.
    const RegionColorConfig region_config;


    //! Mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int average_color_out[CHANNELS_NBR];

    //! Mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    int average_color_in[CHANNELS_NBR];

    //! RGB mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    Rgb_uc average_out;

    //! RGB mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    Rgb_uc average_in;

    //! Sum of component #R, #G, #B of the pixels intside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    Rgb_ui sum_total;
    //! Number of pixels or bytes of #phi.
    const int pxl_nbr_total;

    //! Sum of component #R, #G, #B of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    Rgb_ui sum_out;
    //! Number of pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int pxl_nbr_out;
};

// Definitions

template<typename T>
RegionColorAc::RegionColorAc(ImageSpan32 image1,
                             T&& initial_contour1,
                             const AcConfig& general_config1,           /* optional parameter with AcConfig() */
                             const RegionColorConfig& region_config1) /* optional parameter with RegionColorConfig() */
    : ActiveContour(std::forward<T>(initial_contour1), general_config1),
    image(image1),
    region_config(region_config1),
    pxl_nbr_total(image.size()), pxl_nbr_out(0)
{
    assert( image.width()  == cd_.phi().width() &&
            image.height() == cd_.phi().height()   );

    initialize_sums();
    RegionColorAc::do_specific_cycle1();
}

inline void RegionColorAc::rgb_to_xyz(float R, float G, float B,
                                      float *X, float *Y, float *Z)
{
    R = INV_GAMMA_CORRECTION(R);
    G = INV_GAMMA_CORRECTION(G);
    B = INV_GAMMA_CORRECTION(B);
    *X = (float)(0.4123955889674142161*R + 0.3575834307637148171*G + 0.1804926473817015735*B);
    *Y = (float)(0.2125862307855955516*R + 0.7151703037034108499*G + 0.07220049864333622685*B);
    *Z = (float)(0.01929721549174694484*R + 0.1191838645808485318*G + 0.9504971251315797660*B);
}

inline void RegionColorAc::xyz_to_Lab(float X, float Y, float Z,
                                      float *L, float *a, float *b)
{
    X /= WHITE_POINT_X;
    Y /= WHITE_POINT_Y;
    Z /= WHITE_POINT_Z;
    X = LAB_FUNC(X);
    Y = LAB_FUNC(Y);
    Z = LAB_FUNC(Z);
    *L = 116.f*Y - 16.f;
    *a = 500.f*(X - Y);
    *b = 200.f*(Y - Z);
}

/**
 * Convert CIE XYZ to CIE L*u*v* (CIELUV) with the D65 white point
 *
 * @param L, u, v pointers to hold the result
 * @param X, Y, Z the input XYZ values
 *
 * Wikipedia: http://en.wikipedia.org/wiki/CIELUV_color_space
 */
inline void RegionColorAc::xyz_to_Luv(float X, float Y, float Z,
                                      float *L, float *u, float *v)
{
    float u1, v1, denom;

    denom = X + 15.f*Y + 3.f*Z;

    if( denom > 0.f )
    {
        u1 = (4.f*X) / denom;
        v1 = (9.f*Y) / denom;
    }
    else
    {
        u1 = 0.f;
        v1 = 0.f;
    }

    Y /= WHITE_POINT_Y;
    Y = LAB_FUNC(Y);
    *L = 116.f*Y - 16.f;
    *u = 13.f*(*L)*(u1 - WHITE_POINT_U);
    *v = 13.f*(*L)*(v1 - WHITE_POINT_V);
}

inline void RegionColorAc::rgb_to_Lab(float R, float G, float B,
                                      float *L, float *a, float *b)
{
    float X, Y, Z;
    rgb_to_xyz(R, G, B, &X, &Y, &Z);
    xyz_to_Lab(X, Y, Z, L, a, b);
}

inline void RegionColorAc::rgb_to_Luv(float R, float G, float B,
                                      float *L, float *u, float *v)
{
    float X, Y, Z;
    rgb_to_xyz(R, G, B, &X, &Y, &Z);
    xyz_to_Luv(X, Y, Z, L, u, v);
}

inline void RegionColorAc::rgb_to_color(const Rgb_uc& rgb,
                                        int color[]) const
{
    float cie_color[CHANNELS_NBR];
    const auto cs = region_config.color_space;

    if( cs == ColorSpaceOption::YUV )
    {
        color[0] = ( (  66 * int(rgb.red) + 129 * int(rgb.green) +  25 * int(rgb.blue) + 128) >> 8) +  16; // Y
        color[1] = ( ( -38 * int(rgb.red) -  74 * int(rgb.green) + 112 * int(rgb.blue) + 128) >> 8) + 128; // U
        color[2] = ( ( 112 * int(rgb.red) -  94 * int(rgb.green) -  18 * int(rgb.blue) + 128) >> 8) + 128; // V
    }
    else if ( cs == ColorSpaceOption::Lab )
    {
        rgb_to_Lab(float(rgb.red)/255.f,
                   float(rgb.green)/255.f,
                   float(rgb.blue)/255.f,
                   &cie_color[0],
                   &cie_color[1],
                   &cie_color[2]);

        for( int chl_idx = 0;
             chl_idx < CHANNELS_NBR;
             chl_idx++ )
        {
            color[chl_idx] = int( 255.f * cie_color[chl_idx] );
        }
    }
    else if ( cs == ColorSpaceOption::Luv )
    {
        rgb_to_Luv(float(rgb.red)/255.f,
                   float(rgb.green)/255.f,
                   float(rgb.blue)/255.f,
                   &cie_color[0],
                   &cie_color[1],
                   &cie_color[2]);

        for( int chl_idx = 0;
             chl_idx < CHANNELS_NBR;
             chl_idx++ )
        {
            color[chl_idx] = int( 255.f * cie_color[chl_idx] );
        }
    }
    else
    {
        color[0] = int( rgb.red );
        color[1] = int( rgb.green );
        color[2] = int( rgb.blue );
    }
}

inline void RegionColorAc::compute_external_speed_Fd(ContourPoint& point)
{
    int color[CHANNELS_NBR];

    const Rgb_uc rgb = image.pixel_rgb_at( point.offset() );
    rgb_to_color(rgb, color);

    const int lambda_out = region_config.lambda_out;
    const int lambda_in  = region_config.lambda_in;

    const int* weights   = region_config.weights;
    const int* avg_out   = average_color_out;
    const int* avg_in    = average_color_in;

    int speed_out = 0;
    int speed_in  = 0;

    for (int chl_idx = 0; chl_idx < CHANNELS_NBR; ++chl_idx)
    {
        const int diff_out = color[chl_idx] - avg_out[chl_idx];
        const int diff_in  = color[chl_idx] - avg_in[chl_idx];

        const int w = weights[chl_idx];

        speed_out += w * square(diff_out);
        speed_in  += w * square(diff_in);
    }

    point.set_speed( get_discrete_speed(lambda_out * speed_out - lambda_in * speed_in) );
}

inline void RegionColorAc::do_specific_when_switch(int offset,
                                                   BoundarySwitch ctx_choice)
{
    Rgb_uc rgb = image.pixel_rgb_at(offset);

    if ( ctx_choice == BoundarySwitch::In )
    {
        sum_out.red   -= rgb.red;
        sum_out.green -= rgb.green;
        sum_out.blue  -= rgb.blue;

        pxl_nbr_out--;
    }
    else if ( ctx_choice == BoundarySwitch::Out )
    {
        sum_out.red   += rgb.red;
        sum_out.green += rgb.green;
        sum_out.blue  += rgb.blue;

        pxl_nbr_out++;
    }
}

}

#endif // REGION_COLOR_AC_HPP


//! \class ofeli::RegionColorAc
//! The child class RegionColorAc implements a function to calculate specifically speed \a Fd based on the Chan-Vese model, a region-based energy functional.
//! The regularization of our active contour is performed by a gaussian smoothing of #phi so we are interested uniquely by the external or data dependant term of this energy functional.\n
//! \f$F_{d}=\lambda _{out}\left[ \alpha \left( Y_{out}-C_{outC}\right) ^{2}+ \beta \left( U_{out}-C_{outU}\right) ^{2}+ \gamma \left( V_{out}-C_{outV}\right) ^{2}\right] + \lambda _{in}\left[ \alpha \left( Y_{in}-C_{inY}\right) ^{2}+ \beta \left( U_{in}-C_{inU}\right) ^{2}+ \gamma \left( V_{in}-C_{inV}\right) ^{2}\right]\f$
//!  - \f$F_{d}\f$ : data dependant evolution speed calculated for each point of the active contour, only it sign is used by the algorithm. \n
//!  - \f$Y\f$ : luminance component Y of the (Y,U,V) color space of the current pixel of the active contour. \n
//!  - \f$U\f$ : chrominance component U of the (Y,U,V) color space of the current pixel of the active contour. \n
//!  - \f$V\f$ : chrominance component V of the (Y,U,V) color space of the current pixel of the active contour. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$\lambda _{out}\f$ : weight of the outside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\lambda _{in}\f$ : weight of the inside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\alpha\f$ : weight of the luminance component Y. \n
//!  - \f$\beta\f$ : weight of the chrominance component U. \n
//!  - \f$\gamma\f$ : weight of the chrominance component V.

/**
 * \fn RegionColorAc::RegionColorAc(const unsigned char* img_rgb_data1, int img_width1, int img_height1,
                                            bool has_ellipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                                            bool has_cycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                                            int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1)
 * \param img_rgb_data1 Input pointer on the RGB image data buffer. This buffer must be row-wise and interleaved (R1 G1 B1 R2 G2 B2 ...). Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param has_ellipse1 Boolean to choose the shape of the active contour initialization, \c true for an ellipse or \c false for a rectangle.
 * \param shape_width_ratio1 Width of the shape divided by the image #img_width.
 * \param shape_height_ratio1 Height of the shape divided by the image #img_height.
 * \param center_x_ratio1 X-axis position (or column index) of the center of the shape divided by the image #img_width subtracted by 0.5
 * \param center_y_ratio1 Y-axis position (or row index) of the center of the shape divided by the image #img_height subtracted by 0.5\n
          To have the center of the shape in the image : -0.5 < center_x_ratio1 < 0.5 and -0.5 < center_y_ratio1 < 0.5
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed \a Fint. Passed to #is_cycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambda_out.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambda_in.
 * \param alpha1 Weight of luminance Y. Passed to #alpha.
 * \param beta1 Weight of chrominance U. Passed to #beta.
 * \param gamma1 Weight of chrominance V. Passed to #gamma.
 */

/**
 * \fn RegionColorAc::RegionColorAc(const unsigned char* img_rgb_data1, int img_width1, int img_height1,
                                            const char* phi_init1,
                                            bool has_cycle2_1,
                                            int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                                            int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1)
 * \param img_rgb_data1 Input pointer on the RGB data image buffer. This buffer must be row-wise and interleaved (R1 G1 B1 R2 G2 B2 ...). Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi.
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed \a Fint. Passed to #is_cycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambda_out.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambda_in.
 * \param alpha1 Weight of luminance Y. Passed to #alpha.
 * \param beta1 Weight of chrominance U. Passed to #beta.
 * \param gamma1 Weight of chrominance V. Passed to #gamma.
 */

/**
 * \fn virtual signed char RegionColorAc::compute_external_speed_Fd(ContourPoint& point)
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
