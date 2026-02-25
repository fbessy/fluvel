// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

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
