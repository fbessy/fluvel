#ifndef AC_TYPES_HPP
#define AC_TYPES_HPP

#include <vector>
#include <unordered_set>
#include <limits>

#include "shape.hpp"
#include "contour_data.hpp"

namespace ofeli_ip
{

//! Defines how the algorithm handles degraded or failure cases.
//! Typically used for image segmentation (StopOnFailure)
//! and video tracking (RecoverOnFailure).
enum class FailureHandlingMode {
    StopOnFailure,    //!< Intended for image segmentation.
    RecoverOnFailure  //!< Intended for video tracking.
};

//! \enum Stopping condition status.
enum class StoppingStatus
{
    None,              //!< the active contour is not stopped.
    ListsConverged,    //!< speed <= 0 for all points of Lout and speed >= 0 for all points of Lin
    Hausdorff,         //!< Hausorff distance fallback, available only in StopOnFailure mode.
    MaxIteration,      //!< Maximum number of elementary steps reached, last fallback to avoid
    //!  infinite loop of the method converge().
    EmptyListFailure   //!< One or the both lists is/are empty. The definition of both contiguous lists
    //!  as a boundary is not respected.
};

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

//! BoundarySwitch to perform switch_in or switch_out procedures generically.
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

    ///! Defines how the algorithm handles degraded or failure cases.
    FailureHandlingMode failure_mode;

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
        Na(30), Ns(3),
        failure_mode(FailureHandlingMode::StopOnFailure)
    {
    }

    //! Copy constructor.
    AcConfig(const AcConfig& copied) :
        is_cycle2( copied.is_cycle2 ),
        disk_radius( copied.disk_radius ),
        Na( copied.Na ), Ns( copied.Ns ),
        failure_mode( copied.failure_mode )
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
        this->failure_mode = rhs.failure_mode;

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
                && lhs.Ns            == rhs.Ns
                && lhs.failure_mode  == rhs.failure_mode );
    }

    //! \a Not equal operator overloading.
    friend bool operator!=(const AcConfig& lhs,
                           const AcConfig& rhs)
    {
        return !(lhs == rhs);
    }
};

//! Kernel support to know the geometry limit of the internal kernel.
struct KernelSupport
{
    int min_dx = 0;
    int max_dx = 0;
    int min_dy = 0;
    int max_dy = 0;
};

//! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
struct InternalKernel
{
    std::vector<int> offsets;
    KernelSupport support;

    bool fully_inside(int x, int y,
                      int width, int height) const
    {
        return    x + support.min_dx >= 0
               && x + support.max_dx < width
               && y + support.min_dy >= 0
               && y + support.max_dy < height;
    }
};

//! \struct BoundarySwitchContext to perform generically a switch in or a switch out.
struct BoundarySwitchContext
{
    Contour& active_boundary;
    Contour& adjacent_boundary;
    SpeedValue required_speed_sign;
    PhiValue current_to_adjacent_val;
    PhiValue neighbor_from_region_val;
    PhiValue neighbor_to_boundary_val;
    PhiValue redundant_to_region_val;

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

    //! Stopping condition status.
    StoppingStatus stopping_status;

    //! Constructor.
    EvolutionData(const ContourData& cd)
        : phase_step_count( 0 ),
        step_count( 0 ),
        max_step_count( 5*std::max(cd.phi().width(),
                                   cd.phi().height()) ),
        is_moving( true ),
        previous_step_count(0),
        previous_quantile(0.f),
        hausdorff_quantile(std::numeric_limits<float>().max()),
        centroids_distance(std::numeric_limits<float>().max()),
        stopping_status( StoppingStatus::None )
    {
        l_out_shape.reserve( cd.l_out().capacity() );
        previous_shape.reserve( cd.l_out().capacity() );
        intersection.reserve( cd.l_out().capacity() );
    }

    //! Reset execution state of evolution data. Used for video tracking.
    void resetExecutionState()
    {
        phase_step_count = 0;
        step_count = 0;
        stopping_status = StoppingStatus::None;
    }
};

}

#endif // AC_TYPES_HPP
