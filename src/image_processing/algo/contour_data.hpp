// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include "image_view.hpp"

#include <vector>

namespace fluvel_ip
{

class ContourData
{
public:
    //! Constructor to initialize the contour with one ellipse.
    ContourData(int phiWidth, int phiHeight, Connectivity connectivity = Connectivity::Four);

    //! Constructor to initialize the contour from a grayscale image of the levet-set function
    //! #phi.
    ContourData(ImageView grayscalePhi, Connectivity connectivity = Connectivity::Four);

    //! Constructor to initialize the contour with the both neighbouring boundaries lists of
    //! #outerBoundary and #innerBoundary.
    ContourData(const Contour& outerBoundary, const Contour& innerBoundary, int phiWidth,
                int phiHeight, Connectivity connectivity = Connectivity::Four);

    //! Copy constructor.
    ContourData(const ContourData& contour);

    //! Move constructor.
    ContourData(ContourData&& contour) noexcept;

    //! Wrapper to use directly with offset lists without the need to get the variable #phi.
    Point2D_i coord(size_t offet) const
    {
        return phi_.coord(offet);
    }

    //! Export the boundary list outerBoundary_ as a copied geometric representation.
    ExportedContour export_l_out() const
    {
        return exportContour(outerBoundary_);
    }

    //! Export the boundary list innerBoundary_ as a copied geometric representation.
    ExportedContour export_l_in() const
    {
        return exportContour(innerBoundary_);
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
    //! Getter function for the exterior boundary #outerBoundary.
    Contour& outerBoundary()
    {
        return outerBoundary_;
    }
    const Contour& outerBoundary() const
    {
        return outerBoundary_;
    }
    //! Getter function for the interior boundary #innerBoundary.
    Contour& innerBoundary()
    {
        return innerBoundary_;
    }
    const Contour& innerBoundary() const
    {
        return innerBoundary_;
    }

    bool empty() const
    {
        return outerBoundary_.empty() || innerBoundary_.empty();
    }

    bool isValid() const;

    Connectivity connectivity() const
    {
        return connectivity_;
    }

    bool isTrivialDomain() const
    {
        return phi_.width() < 2 || phi_.height() < 2;
    }

private:
    //! Initializes the contour *this with one ellipse. It is performed when the simplest
    //! constructor is called or when one or both boundary lists is/are empty.
    void defineFromEllipse();

    //! Eliminates redundant points to maintain a contiguous boundary.
    void eliminateRedundantPoints(Contour& boundary, PhiValue regionValue);

    //! To use defineFromEllipse and eliminateRedundantPoints
    //! in ActiveContour.
    friend class ActiveContour;

    //! Checks if a given point is redundant to define a boundary, i.e. if no neighbors have a
    //! different phi value sign comparing to the given point.
    bool isRedundant(const ContourPoint& point) const;

    //! Allocate lists.
    void allocateLists();

    //! Defines the boundary lists and phi from a binary phi.
    void defineListsAndPhiFromBinaryPhi();

    //! Defines the #phi level-set function from the boundary lists #outerBoundary and
    //! #innerBoundary.
    void definePhiFromLists();

    //! Performs a flood fill algorithm for the method #definePhiFromLists().
    void floodFill(const Point2D_i& seed, PhiValue targetValue, PhiValue replacementValue);

    //! Eliminates redundant points for the both lists to maintain a contiguous boundary.
    void eliminateRedundantPointsIfNeeded();

    //! Export a boundary list as a copied geometric representation.
    std::vector<Point2D_i> exportContour(const Contour& boundary) const;

    template <typename T> static bool hasDuplicates(const std::vector<T>& v);

    //! Discrete level-set function with only 4 PhiValue possible.
    DiscreteLevelSet phi_;

    //! List of points representing the exterior boundary (called Lout in the reference paper).
    Contour outerBoundary_;
    //! List of points representing the interior boundary (called Lin in the reference paper).
    Contour innerBoundary_;

    //! Pixel connectivity of the neighborhood to define ContourData (4- or 8-connected).
    const Connectivity connectivity_;
};

template <typename T> bool ContourData::hasDuplicates(const std::vector<T>& v)
{
    for (std::size_t i = 0; i < v.size(); ++i)
        for (std::size_t j = i + 1; j < v.size(); ++j)
            if (v[i] == v[j])
                return true;
    return false;
}

} // namespace fluvel_ip
