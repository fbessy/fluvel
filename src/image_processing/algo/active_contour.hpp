/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.F
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

#ifndef ACTIVE_CONTOUR_HPP
#define ACTIVE_CONTOUR_HPP

#include <vector>
#include <unordered_set>
#include <cstddef>
#include <iostream>
#include <cmath>

#include "contour_data.hpp"
#include "shape.hpp"
#include "point.hpp"
#include "point_hash.hpp"

namespace ofeli_ip
{

constexpr size_t INITIAL_SPEED_ARRAY_ALLOC_SIZE = 10000u;

//! Internal phase state of the active contour.
//! A logical cycle consists of Phase Cycle1 followed by Phase Cycle2,
//! as described in the reference paper.
enum class PhaseState
{
    Cycle1,       //!< Phase 1 (called "Cycle 1" in the reference paper)
    Cycle2,       //!< Phase 2 (called "Cycle 2" in the reference paper)
    FinalCycle2,  //!< Final Phase 2 before termination
    Stopped
};

//! \enum Stopping condition status.
enum class StoppingStatus
{
    None,           //!< Maximum number of elementary steps reached.
    EmptyList,      //!< One or the both lists is/are empty. The definition of both contiguous lists
                    //!  as a boundary is not respected.
    MaxIteration,   //!< Maximum number of elementary steps reached.
    ListsStopped,   //!< speed <= 0 for all points of l_out and speed >= 0 for all points of l_in
    Hausdorff       //!< Hausorff distance convergence.
};

enum class BoundarySwitch
{
    In,
    Out
};

//! \class AcConfig
//! Active contour configuration
struct AcConfig
{
    //! Boolean egals to \c true to have the curve smoothing, evolutions in the cycle 2 with the internal speed Fint.
    bool is_cycle2;

    //!  Disk radius for the curve smoothing.
    int disk_radius;

    //! Maximum number of times the active contour can evolve in a cycle 1 with \a Fd speed.
    int Na;

    //! Maximum number of times the active contour can evolve in a cycle 2 with \a Fint speed.
    int Ns;

    //! Normalize values of a configuration.
    void normalize()
    {
        if( disk_radius < 1 )
            disk_radius = 1;

        if( Na < 1 )
            Na = 1;

        if( Ns < 1 )
            Ns = 1;
    }

    //! Default constructor.
    AcConfig() : is_cycle2(true),
                 disk_radius(2),
                 Na(30), Ns(3)
    {
    }

    //! Copy constructor.
    AcConfig(const AcConfig& copied) :
        is_cycle2( copied.is_cycle2 ),
        disk_radius( copied.disk_radius ),
        Na( copied.Na ), Ns( copied.Ns )
    {
        this->normalize();
    }

    //! Copy assignement operator.
    AcConfig& operator=(const AcConfig& rhs)
    {
        this->is_cycle2 = rhs.is_cycle2;
        this->disk_radius = rhs.disk_radius;
        this->Na = rhs.Na;
        this->Ns = rhs.Ns;

        this->normalize();

        return *this;
    }

    //! \a Equal operator overloading.
    friend bool operator==(const AcConfig& lhs,
                           const AcConfig& rhs)
    {
        return (    lhs.is_cycle2     == rhs.is_cycle2
                 && lhs.disk_radius   == rhs.disk_radius
                 && lhs.Na            == rhs.Na
                 && lhs.Ns            == rhs.Ns );
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const AcConfig& lhs,
                           const AcConfig& rhs)
    {
        return !(lhs == rhs);
    }
};

//! \struct BoundarySwitchContext to perform generically a switch in or a switch out.
struct BoundarySwitchContext
{
    ContourList& scanned_boundary;
    ContourList& adjacent_boundary;
    SpeedValue target_direction;
    PhiValue adjacent_phi_val;
    PhiValue neighbor_region_phi_val;
    PhiValue neighbor_boundary_phi_val;
    PhiValue region_redundant_phi_val;

    static BoundarySwitchContext make_switch_in(ContourData& cd)
    {
        return {
            cd.l_out(),
            cd.l_in(),
            SpeedValue::GoOutward,
            PhiValue::InteriorBoundary,
            PhiValue::OutsideRegion,
            PhiValue::ExteriorBoundary,
            PhiValue::InsideRegion
        };
    }

    static BoundarySwitchContext make_switch_out(ContourData& cd)
    {
        return {
            cd.l_in(),
            cd.l_out(),
            SpeedValue::GoInward,
            PhiValue::ExteriorBoundary,
            PhiValue::InsideRegion,
            PhiValue::InteriorBoundary,
            PhiValue::OutsideRegion
        };
    }
};

//! \class EvolutionData
//! Holds the evolution data of the active contour.
struct EvolutionData
{
    //! Iterations number in a cycle (cycle 1 or cycle 2). It is set to 0 at the end of one cycle.
    int phase_step_count;

    //! Total number of iterations the active contour has evolved from the initial contour.
    int step_count;

    //! Maximum number of times the active contour can evolve.
    const int max_step_count;

    //! Boolean egals to true if the active contour evolves in one way (at least) in cycle 1.
    bool is_moving;

    //! l_out shape at the end of the cycle 2.
    Shape l_out_shape;

    //! l_out shape at the end of the previous cycle 2.
    Shape previous_shape;

    //! Total number of iterations the active contour has evolved from the initial contour
    //! at the end of the previous cycle 2.
    int previous_step_count;

    //! Hausdorff quantile
    //! at the end of the previous cycle 2.
    float previous_quantile;

    //! Hausdorff quantile
    //! at the end of the cycle 2. It is a normalized value, divided by the diagonal size of #phi, in percent.
    float hausdorff_quantile;

    //! Centroids gap between #l_out_shape and previous_shape#. It is a normalized value, divided by the diagonal size of #phi, in percent.
    float centroids_distance;

    //! Intersection, i.e common points between #l_out_shape and #previous_shape.
    std::unordered_set<Point2D_i>intersection;

    //! Constructor.
    EvolutionData(const ContourData& cd)
        : phase_step_count( 0 ),
          step_count( 0 ),
          max_step_count( 5*std::max(cd.phi().width(),
                                     cd.phi().height()) ),
          is_moving( true ),

          l_out_shape( cd.preallocation_size() ),
          previous_shape( cd.preallocation_size() ),
          previous_step_count(0),
          previous_quantile(0.f),
          hausdorff_quantile(std::numeric_limits<float>().max()),
          centroids_distance(std::numeric_limits<float>().max())
    {
        intersection.reserve( cd.preallocation_size() );
    }

    //! Reinitialize evolution data. Used for video tracking.
    void reinitialize()
    {
        phase_step_count = 0;
        step_count = 0;
    }
};

class ActiveContour
{

public :

    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and #l_out)
    //! with a copy semantic.
    ActiveContour(const ContourData& initial_contour,
                  const AcConfig& config1);

    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and #l_out)
    //! with a move semantic.
    ActiveContour(ContourData&& initial_contour,
                  const AcConfig& config1);

    //! Destructor.
    virtual ~ActiveContour() {}

    //! Runs or evolves the active contour until it reaches a terminal state.
    void converge();

    //! The active contour evolves to one iteration.
    void step();

    //! Runs the active contour for a fixed number of full cycles (cycle1 + cycle2).
    //! Ensures the contour is geometrically stable at the end of each cycle2 (used for video tracking).
    void run_cycles(int n_cycles);

    //! Getter for the discrete level-set function with only 4 PhiValue possible.
    const DiscreteLevelSet& phi() const { return cd_.phi(); }

    //! Getter for the list of offset points representing the exterior boundary.
    const ContourList& l_out() const { return cd_.l_out(); }
    //! Getter for the list of offset points representing the interior boundary.
    const ContourList& l_in() const { return cd_.l_in(); }

    //! Getter method for #state.
    PhaseState state() const { return state_; }

    //! Gets if the active contour reaches the final state.
    bool stopped() const;

    //! Gets if the active contour reaches the final state successfully.
    bool converged() const;

    //! Getter method for #stopping_status.
    StoppingStatus stop_reason() const { return stopping_status_; }

    //! Getter method for #step_count.
    int step_count() const { return ed_.step_count; }

    //! Getter method for #hausdorff_quantile.
    float hausdorff_quantile() const { return ed_.hausdorff_quantile; }

    //! Getter method for #centroids_distance.
    float centroids_distance() const { return ed_.centroids_distance; }

protected :

    //! Reinitialize the active contour, used for video tracking.
    void reinitialize();

    //! Gets a discrete speed.
    static SpeedValue get_discrete_speed(int speed);

    //! Gives the square of a value.
    template <typename T>
    static T square(const T& value);

    //! Representation data of the active contour
    //! (discret level-set function phi, Lin and Lout)
    ContourData cd_;

private :

    //! Runs the active contour for a fixed number of elementary steps.
    //! Intended for incremental updates (e.g. video tracking).
    void run_steps(int n_steps);

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void step_cycle1();

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void step_cycle2();

    //! It selects the context.
    void select_context(BoundarySwitch ctx_choice);

    //! Get the selected context.
    const BoundarySwitchContext& context() { return *ctx_; }

    //! Performs one directional topological update step (in or out).
    //! The step includes velocity computation, boundary switching,
    //! and adjacent boundary cleanup.
    bool directional_substep(BoundarySwitch ctx_choice);

    //! Computes the speed for all points of a boundary list #l_out or #l_in.
    void compute_speed(ContourList& boundary);

    //! Computes the external speed Fd for all points of a boundary list #l_out or #l_in.
    void compute_external_speed_Fd(ContourList& boundary);

    //! Computes the external speed \a Fd for a current point (\a x,\a y) of #l_out or #l_in.
    virtual void compute_external_speed_Fd(ContourPoint& point);

    //! Computes the internal speed  Fint for all points of a boundary list #l_out or #l_in.
    void compute_internal_speed_Fint(ContourList& boundary);

    //! Computes the internal speed  Fint for a current point (\a x,\a y) of #l_out or #l_in.
    void compute_internal_speed_Fint(ContourPoint& point);

    //! Build precomputed disk-shaped kernel offsets for internal smoothing (Fint).
    void build_internal_kernel_offsets();

    //! Generic method to handle outward / inward local movement of a current boundary point (of #l_out or #l_in) and to switch it from one boundary list to the other.
    void switch_boundary_point(ContourPoint& point);

    //! Second procedure for generic method switch_boundary_point.
    void add_region_neighbor(int neighbor_offset,
                             int neighbor_x);

    //! Eliminates redundant points to maintain a contiguous boundary.
    void eliminate_redundant_points(ContourList& boundary,
                                    PhiValue region_value);

    //! Specific step for each iteration in cycle 1.
    virtual void do_specific_cycle1() { }

    //! Specific step when switch in or a switch out procedure is performed.
    virtual void do_specific_when_switch(int,
                                         BoundarySwitch) { }

    //! Stops the active contour and puts it in a terminal state.
    //! After this call, step(), converge(), and run_cycles() have no effect.
    void stop(StoppingStatus reason);

    //! Checks generic condition at each iteration in both cycles to determine state Stopped.
    void check_stopping_condition();

    //! Calculates the active contour state at the end of each iteration of a cycle 1.
    void check_state_step1();

    //! Updates the active contour state at the end of a cycle 2.
    void update_state_cycle2();

    //! Computes the shapes intersection between #ed.l_out_shape and #ed.previous_shape
    //! to speed up the hausdorff distance computation.
    void calculate_shapes_intersection();

    //! To transformate active contour data point to the points for the Hausdorff distance.
    static Point2D_i from_ContourPoint(const ContourPoint& point,
                                     int grid_width);

    //! Generic configuration of the active contour.
    const AcConfig config_;

    //! Number of iterations in one cycle1-cycle2.
    int steps_per_cycle_;

    //! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
    std::vector<int> internal_kernel_offsets_;

    //! Evolution state of the active contour given at a current iteration.
    //!  There are 4 states : Cycle1, Cycle2, FinalCycle2 and Stopped.
    PhaseState state_;

    //! Stopping condition status.
    StoppingStatus stopping_status_;

    //! BoundarySwitchContext for the procedure switch_in.
    BoundarySwitchContext ctx_in_;

    //! BoundarySwitchContext for the procedure switch_out.
    BoundarySwitchContext ctx_out_;

    //! A selector (pointer) to perform a switch generically. It pointes to ctx_in_ or ctx_out_.
    BoundarySwitchContext* ctx_;

    //! Evolution data of the active contour used by #check_state_step1() and
    //! #update_state_cycle2() to calculate #state.
    EvolutionData ed_;

    //! Boolean egals to true if the cycle 2 stopping condition (based on hausdorff distance)
    //! is performed.
    bool is_cycle2_condition_;

    //! Temporary points to add after each scan of the list #l_in or #l_out.
    ContourList points_to_append_;
};

template <typename T>
inline T ActiveContour::square(const T& value)
{
    return value*value;
}

inline SpeedValue ActiveContour::get_discrete_speed(int speed)
{
    if (speed < 0) return SpeedValue::GoInward;
    if (speed > 0) return SpeedValue::GoOutward;
    return SpeedValue::NoMove;
}

}

#endif // ACTIVE_CONTOUR_HPP



//! \class ofeli::ActiveContour
//! The class ActiveContour contains the implementation of the Fast-Two-Cycle (FTC) algorithm of Shi and Karl as it describes in their article "A real-time algorithm for the approximation of level-set based curve evolution" published in IEEE Transactions on Image Processing in may 2008.
//! This class should be never instantiated because the function to calculate the external speed \a Fd is too general. The 3 classes #ofeli::ACwithoutEdges, #ofeli::ACwithoutEdgesYUV and #ofeli::GeodesicAC, inherit of the class ActiveContour, with for each class it own implementation of this function.

/**
 * \fn ActiveContour::ActiveContour(const unsigned char* img_data1, int img_width1, int img_height1,
                                    bool has_ellipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                                    bool has_cycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1)
 * \param img_data1 Input pointer on the image data buffer. This buffer must be row-wise, except if you define the macro COLUMN_WISE_IMAGE_DATA_BUFFER. Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param has_ellipse1 Boolean to choose the shape of the active contour initialization, \c true for an ellipse or \c false for a rectangle.
 * \param shape_width_ratio1 Width of the shape divided by the image #img_width.
 * \param shape_height_ratio1 Height of the shape divided by the image #img_height.
 * \param center_x_ratio1 X-axis position or column index of the center of the shape divided by the image #img_width subtracted by 0.5.
 * \param center_y_ratio1 Y-axis position or row index of the center of the shape divided by the image #img_height subtracted by 0.5.\n
          To have the center of the shape into the image : -0.5 < center_x1 < 0.5 and -0.5 < center_y_ratio1 < 0.5.
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed Fint. Passed to #is_cycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of maximum iterations the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of maximum iterations the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 */

/**
 * \fn ActiveContour::ActiveContour(const unsigned char* img_data1, int img_width1, int img_height1,
                                    const char* phi_init1,
                                    bool has_cycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1)
 * \param img_data1 Input pointer on the image data buffer. This buffer must be row-wise, except if you define the macro COLUMN_WISE_IMAGE_DATA_BUFFER. Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi. #phi is checked and cleaned if needed after so \a phi_init1 can be a binarized buffer with 1 for outside region and -1 for inside region to simplify your task.
 * \param has_cycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed Fint. Passed to #is_cycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of maximum iterations the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of maximum iterations the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 */

/**
 * \fn void ActiveContour::switch_in(ofeli::list<int>::iterator l_out_point)
 * \param l_out_point iterator which contains offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn void ActiveContour::switch_out(ofeli::list<int>::iterator l_in_point)
 * \param l_in_point which contains offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn signed char ActiveContour::compute_internal_speed_Fint(int offset)
 * \param offset offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 * \return Fint, the internal speed for the regularization of the active contour used in the cycle 2
 */

/**
 * \fn virtual signed char ActiveContour::compute_external_speed_Fd(ContourPoint& point)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 * \return Fd, the external speed for the data dependant evolution of the active contour used in the cycle 1
 */

/**
 * \fn bool ActiveContour::isRedundantLinPoint(int offset) const
 * \param offset offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 * \return \c true if the point (\a x,\a y) of #l_in is redundant, otherwise, \c false
 */

/**
 * \fn bool ActiveContour::isRedundantLoutPoint(int offset) const
 * \param offset offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 * \return \c true if the point (\a x,\a y) of #l_out is redundant, otherwise, \c false
 */

/**
 * \fn virtual void ActiveContour::updates_for_means_in2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual void ActiveContour::updates_for_means_out2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \example interface
 * \code
 * // It interfaces the algorithm in order to display each iteration
 *
 * #include "ac_withoutedges.hpp"
 * #include "List.hpp"
 * #include <iostream>
 *
 * int offset // offset of the current pixel
 * unsigned char I // intensity of the current pixel
 *
 * // Lout displayed in blue
 * unsigned char Rout = 0;
 * unsigned char Gout = 0;
 * unsigned char Bout = 255;
 *
 * // Lin displayed in red
 * unsigned char Rin = 255;
 * unsigned char Gin = 0;
 * unsigned char Bin = 0;
 *
 *
 * ofeli::ACwithoutEdges ac(img1, img_width1, img_height1);
 *
 *
 * -----------------------------   Display the initial active contour   -----------------------------
 *
 * const ofeli::list<int>* Lout = &ac.l_out();
 * const ofeli::list<int>* Lin = &ac.l_in();
 *
 * // put the color of lists into the displayed buffer
 * for( auto it = Lout->get_begin(); it != Lout->get_end(); ++it )
 * {
 *     offset = *it*3;
 *
 *     img_displayed[offset] = Rout;
 *     img_displayed[offset+1] = Gout;
 *     img_displayed[offset+2] = Bout;
 * }
 *
 * for( auto it = Lin->get_begin(); it != Lin->get_end(); ++it )
 * {
 *     offset = *it*3;
 *
 *     img_displayed[offset] = Rin;
 *     img_displayed[offset+1] = Gin;
 *     img_displayed[offset+2] = Bin;
 * }
 *
 * // paint event, refresh the widget witch display the buffer img_displayed
 * update();
 *
 * // display the position of the active contour in the standard output
 * std::cout << ac << std::endl;
 *
 * --------------------------------------------------------------------------------------------------
 *
 *
 * // Loop for the evolution of the active contour
 * while( !ac.get_is_stopped() )
 * {
 *     // erase the previous lists Lout1 and Lin1 of the displayed buffer
 *     for( auto it = Lout->get_begin(); it != Lout->get_end(); ++it )
 *     {
 *         offset = *it;
 *
 *         I = img1[offset];
 *
 *         img_displayed[3*offset] = I;
 *         img_displayed[3*offset+1] = I;
 *         img_displayed[3*offset+2] = I;
 *     }
 *
 *     for( auto it = Lin->get_begin(); it != Lin->get_end(); ++it )
 *     {
 *         offset = *it;
 *
 *         I = img1[offset];
 *
 *         img_displayed[3*offset] = I;
 *         img_displayed[3*offset+1] = I;
 *         img_displayed[3*offset+2] = I;
 *     }
 *
 *     // to evolve the active contour of one iteration or one step
 *     ++ac;
 *
 *     // to get the temporary result
 *     Lout = &ac.l_out();
 *     Lin = &ac.l_in();
 *
 *     // put the color of lists into the displayed buffer
 *     for( auto it = Lout->get_begin(); it != Lout->get_end(); ++it )
 *
 *         offset = *it*3;
 *
 *         img_displayed[offset] = Rout;
 *         img_displayed[offset+1] = Gout;
 *         img_displayed[offset+2] = Bout;
 *     }
 *
 *     for( auto it = Lin->get_begin(); it != Lin->get_end(); ++it )
 *
 *         offset = *it*3;
 *
 *         img_displayed[offset] = Rin;
 *         img_displayed[offset+1] = Gin;
 *         img_displayed[offset+2] = Bin;
 *     }
 *
 *     // paint event, refresh the widget witch display the buffer img_displayed
 *     update();
 * }
 *
 * \endcode
 */
