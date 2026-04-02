// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "image_view.hpp"

#include <vector>

namespace fluvel_ip
{

class ContourData
{
public:
    //! Constructor to initialize the contour with one ellipse.
    ContourData(int phi_width, int phi_height, Connectivity connectivity = Connectivity::Four);

    //! Constructor to initialize the contour from a grayscale image of the levet-set function
    //! #phi.
    ContourData(ImageView grayscale_phi, Connectivity connectivity = Connectivity::Four);

    //! Constructor to initialize the contour with the both neighbouring boundaries lists of
    //! #l_out and #l_in.
    ContourData(const Contour& l_out, const Contour& l_in, int phi_width, int phi_height,
                Connectivity connectivity = Connectivity::Four);

    //! Copy constructor.
    ContourData(const ContourData& contour);

    //! Move constructor.
    ContourData(ContourData&& contour) noexcept;

    //! Wrapper to use directly with offset lists without the need to get the variable #phi.
    Point2D_i coord(size_t offet) const
    {
        return phi_.coord(offet);
    }

    //! Export the boundary list l_out_ as a copied geometric representation.
    ExportedContour export_l_out() const
    {
        return export_contour(l_out_);
    }

    //! Export the boundary list l_in_ as a copied geometric representation.
    ExportedContour export_l_in() const
    {
        return export_contour(l_in_);
    }

    //! Getter function for the discrete level-set function #phi.
    DiscreteLevelSet& phi()
    {
        return phi_;
    }
    const DiscreteLevelSet& phi() const
    {
        return phi_;
    }
    //! Getter function for the exterior boundary #l_out.
    Contour& l_out()
    {
        return l_out_;
    }
    const Contour& l_out() const
    {
        return l_out_;
    }
    //! Getter function for the interior boundary #l_in.
    Contour& l_in()
    {
        return l_in_;
    }
    const Contour& l_in() const
    {
        return l_in_;
    }

    bool empty() const
    {
        return l_out_.empty() || l_in_.empty();
    }

    bool is_valid() const;

    Connectivity connectivity() const
    {
        return connectivity_;
    }

private:
    //! Initializes the contour *this with one ellipse. It is performed when the simplest
    //! constructor is called or when one or both boundary lists is/are empty.
    void define_from_ellipse();

    //! Eliminates redundant points to maintain a contiguous boundary.
    void eliminateRedundantPoints(Contour& boundary, PhiValue region_value);

    //! To use define_from_ellipse and eliminateRedundantPoints
    //! in ActiveContour.
    friend class ActiveContour;

    //! Checks if a given point is redundant to define a boundary, i.e. if no neighbors have a
    //! different phi value sign comparing to the given point.
    bool is_redundant(const ContourPoint& point) const;

    //! Allocate lists.
    void allocate_lists();

    //! Defines the boundary lists and phi from a binary phi.
    void define_lists_and_phi_from_binary_phi();

    //! Defines the #phi level-set function from the boundary lists #l_out and #l_in.
    void define_phi_from_lists();

    //! Performs a flood fill algorithm for the method #define_phi_from_lists().
    void flood_fill(const Point2D_i& seed, PhiValue target_value, PhiValue replacement_value);

    //! Eliminates redundant points for the both lists to maintain a contiguous boundary.
    void eliminate_redundant_points_if_needed();

    //! Export a boundary list as a copied geometric representation.
    std::vector<Point2D_i> export_contour(const Contour& boundary) const;

    template <typename T> static bool has_duplicates(const std::vector<T>& v);

    //! Discrete level-set function with only 4 PhiValue possible.
    DiscreteLevelSet phi_;

    //! List of points representing the exterior boundary (called Lout in the reference paper).
    Contour l_out_;
    //! List of points representing the interior boundary (called Lin in the reference paper).
    Contour l_in_;

    //! Pixel connectivity of the neighborhood to define ContourData (4- or 8-connected).
    const Connectivity connectivity_;
};

template <typename T> bool ContourData::has_duplicates(const std::vector<T>& v)
{
    for (std::size_t i = 0; i < v.size(); ++i)
        for (std::size_t j = i + 1; j < v.size(); ++j)
            if (v[i] == v[j])
                return true;
    return false;
}

} // namespace fluvel_ip
