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
#include <limits>

#include "contour_data.hpp"
#include "shape.hpp"
#include "point.hpp"
#include "point_hash.hpp"
#include "ac_types.hpp"

namespace ofeli_ip
{

constexpr size_t INITIAL_SPEED_ARRAY_ALLOC_SIZE = 10000u;

class ActiveContour
{

public :

    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and #l_out)
    //! with a copy semantic.
    ActiveContour(const ContourData& initial_state,
                  const AcConfig& config);

    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and #l_out)
    //! with a move semantic.
    ActiveContour(ContourData&& initial_state,
                  const AcConfig& config);

    //! Destructor.
    virtual ~ActiveContour() {}

    //! Handles a failure case.
    void handle_failure();

    //! Runs or evolves the active contour until it reaches a terminal state.
    void converge();

    //! The active contour evolves to one iteration.
    void step();

    //! Runs the active contour for a fixed number of full cycles (cycle1 + cycle2).
    //! Ensures the contour is geometrically stable at the end of each cycle2 (used for video tracking).
    void run_cycles(int n_cycles);


    //! Export the boundary list l_out_ as a copied geometric representation.
    ExportedContour export_l_out() const { return cd_.export_l_out(); }

    //! Export the boundary list l_in_ as a copied geometric representation.
    ExportedContour export_l_in() const  { return cd_.export_l_in(); }

    //! Getter for the discrete level-set function with only 4 PhiValue possible.
    const DiscreteLevelSet& phi() const { return cd_.phi(); }

    //! Getter for the list of offset points representing the exterior boundary.
    const Contour& l_out() const { return cd_.l_out(); }
    //! Getter for the list of offset points representing the interior boundary.
    const Contour& l_in() const { return cd_.l_in(); }

    //! Getter method for #state.
    PhaseState state() const { return state_; }

    //! Gets if the active contour is with the initial state.
    bool is_initial() const { return step_count() == 0; }

    //! Gets if the active contour reaches the final state.
    bool is_stopped() const { return state_ == PhaseState::Stopped; }

    //! Getter method for #stopping_status.
    StoppingStatus stop_reason() const { return ed_.stopping_status; }

    //! Getter method for #step_count.
    int step_count() const { return ed_.step_count; }

    //! Getter method for #hausdorff_quantile.
    float hausdorff_quantile() const { return ed_.hausdorff_quantile; }

    //! Getter method for #centroids_distance.
    float centroids_distance() const { return ed_.centroids_distance; }

protected :

    //! restart the active contour, used for video tracking.
    void restart();

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
    void compute_speed(Contour& boundary);

    //! Computes the external speed Fd for all points of a boundary list #l_out or #l_in.
    void compute_external_speed_Fd(Contour& boundary);

    //! Computes the external speed \a Fd for a current point (\a x,\a y) of #l_out or #l_in.
    virtual void compute_external_speed_Fd(ContourPoint& point);

    //! Computes the internal speed  Fint for all points of a boundary list #l_out or #l_in.
    void compute_internal_speed_Fint(Contour& boundary);

    //! Computes the internal speed  Fint for a current point (\a x,\a y) of #l_out or #l_in.
    void compute_internal_speed_Fint(ContourPoint& point);

    //! Generic method to handle outward / inward local movement of a current boundary point (of #l_out or #l_in) and to switch it from one boundary list to the other.
    void switch_boundary_point(ContourPoint& point);

    //! Promote a neighboring region point to boundary.
    void promote_region_to_boundary(int nx, int ny);

    //! Specific step for each iteration in cycle 1.
    virtual void do_specific_cycle1() { }

    //! Specific step when switch in or a switch out procedure is performed.
    virtual void do_specific_when_switch(const ContourPoint& point,
                                         BoundarySwitch) { }

    //! Stops the active contour and puts it in a terminal state.
    //! After this call, step(), converge(), and run_cycles() have no effect.
    void stop();

    //! Check if iteration limit is not reached.
    void enforce_iteration_limit();

    //! Calculates the active contour state at the end of each iteration of a cycle 1.
    void check_state_step1();

    //! Updates the active contour state at the end of a cycle 2.
    void update_state_cycle2();

    //! Check an internal condition, based on the contour state rather than image data,
    //! to stop the algorithm if the nominal convergence condition is not reached.
    void check_hausdorff_stopping_condition();

    //! Computes the shapes intersection between #ed.l_out_shape and #ed.previous_shape
    //! to speed up the hausdorff distance computation.
    void calculate_shapes_intersection();

    //! Build kernel offsets.
    static InternalKernel make_internal_kernel_offsets(int disk_radius,
                                                       int grid_width);

    //! To transformate active contour data point to the points for the Hausdorff distance.
    static Point2D_i from_ContourPoint(const ContourPoint& point);

    //! Generic configuration of the active contour.
    const AcConfig config_;

    //! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
    const InternalKernel internal_kernel_;

    //! BoundarySwitchContext for the procedure switch_in.
    const BoundarySwitchContext ctx_in_;

    //! BoundarySwitchContext for the procedure switch_out.
    const BoundarySwitchContext ctx_out_;

    //! A selector (pointer) to perform a switch generically. It pointes to ctx_in_ or ctx_out_.
    const BoundarySwitchContext* ctx_;

    //! Temporary points to add after each scan of the list #l_in or #l_out.
    Contour active_boundary_staging_;

    //! Number of iterations in one cycle1-cycle2.
    int steps_per_cycle_;

    //! Evolution state of the active contour given at a current iteration.
    //!  There are 4 states : Cycle1, Cycle2, FinalCycle2 and Stopped.
    PhaseState state_;

    //! Evolution data of the active contour used by #check_state_step1() and
    //! #update_state_cycle2() to calculate #state.
    EvolutionData ed_;
};

inline Point2D_i ActiveContour::from_ContourPoint(const ContourPoint& point)
{
    return { point.x(), point.y() };
}

namespace speed_value
{

//! Gets a discrete speed.
constexpr SpeedValue get_discrete_speed(int speed);

constexpr SpeedValue get_discrete_speed(int speed)
{
    if (speed < 0) return SpeedValue::GoInward;
    if (speed > 0) return SpeedValue::GoOutward;
    return SpeedValue::NoMove;
}

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
