// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include "image_view.hpp"

#include <vector>

namespace fluvel_ip
{

/**
 * @brief Represents the full data structure of an active contour.
 *
 * This class encapsulates:
 * - the discrete level-set function (@c phi),
 * - the outer boundary (Lout),
 * - the inner boundary (Lin).
 *
 * It provides multiple initialization modes:
 * - automatic ellipse initialization,
 * - initialization from a binary image,
 * - initialization from explicit boundary lists.
 *
 * It also ensures consistency between the level-set representation
 * and the boundary lists.
 */
class ContourData
{
public:
    /**
     * @brief Construct a contour initialized with a default ellipse.
     *
     * @param phiWidth Width of the domain.
     * @param phiHeight Height of the domain.
     * @param connectivity Neighborhood connectivity (4 or 8).
     */
    ContourData(int phiWidth, int phiHeight, Connectivity connectivity = Connectivity::Four);

    /**
     * @brief Construct contour data from a binary initialization mask.
     *      * The input image is interpreted as a binary region map.
     * Pixel values greater than or equal to 128 are considered inside,
     * lower values are considered outside.
     *      * Pixel values are fully copied into the internal phi representation.
     * The provided ImageView is not stored and is only accessed during
     * construction, therefore temporary or short-lived ImageView instances
     * are safe to pass.
     *      * @param binaryMask Input binary mask used to initialize the contour.
     * @param connectivity Neighborhood connectivity (4 or 8).
     */
    ContourData(const ImageView& binaryMask, Connectivity connectivity = Connectivity::Four);

    /**
     * @brief Construct a contour from explicit boundary lists.
     *
     * @param outerBoundary Exterior boundary (Lout).
     * @param innerBoundary Interior boundary (Lin).
     * @param phiWidth Width of the domain.
     * @param phiHeight Height of the domain.
     * @param connectivity Neighborhood connectivity (4 or 8).
     */
    ContourData(ContourPoints outerBoundary, ContourPoints innerBoundary, int phiWidth,
                int phiHeight, Connectivity connectivity = Connectivity::Four);

    /**
     * @brief Copy constructor.
     */
    ContourData(const ContourData& contour);

    /**
     * @brief Move constructor.
     */
    ContourData(ContourData&& contour) noexcept;

    /**
     * @brief Convert an offset index into 2D coordinates.
     *
     * @param offet Linear offset in the phi buffer.
     * @return Corresponding 2D coordinates.
     */
    Point2D_i coord(size_t offet) const
    {
        return phi_.coord(offet);
    }

    /**
     * @brief Export the outer boundary as a geometric contour.
     *
     * @return A vector of 2D points representing Lout.
     */
    Contour exportOuterBoundary() const
    {
        return toContour(outerBoundary_);
    }

    /**
     * @brief Export the inner boundary as a geometric contour.
     *
     * @return A vector of 2D points representing Lin.
     */
    Contour exportInnerBoundary() const
    {
        return toContour(innerBoundary_);
    }

    /**
     * @brief Access the discrete level-set function.
     */
    DiscreteLevelSet& phi()
    {
        return phi_;
    }

    /**
     * @brief Const access to the discrete level-set function.
     */
    const DiscreteLevelSet& phi() const
    {
        return phi_;
    }

    /**
     * @brief Access the outer boundary (Lout).
     */
    ContourPoints& outerBoundary()
    {
        return outerBoundary_;
    }

    /**
     * @brief Const access to the outer boundary (Lout).
     */
    const ContourPoints& outerBoundary() const
    {
        return outerBoundary_;
    }

    /**
     * @brief Access the inner boundary (Lin).
     */
    ContourPoints& innerBoundary()
    {
        return innerBoundary_;
    }

    /**
     * @brief Const access to the inner boundary (Lin).
     */
    const ContourPoints& innerBoundary() const
    {
        return innerBoundary_;
    }

    /**
     * @brief Check if one of the boundary lists is empty.
     *
     * @return True if either Lout or Lin is empty.
     */
    bool empty() const
    {
        return outerBoundary_.empty() || innerBoundary_.empty();
    }

    /**
     * @brief Check if the contour data is valid.
     *
     * This typically ensures consistency between phi and the boundary lists.
     */
    bool isValid() const;

    /**
     * @brief Get the connectivity used by this contour.
     */
    Connectivity connectivity() const
    {
        return connectivity_;
    }

    /**
     * @brief Check if the domain is too small to be meaningful.
     *
     * @return True if width < 2 or height < 2.
     */
    bool isTrivialDomain() const
    {
        return phi_.width() < 2 || phi_.height() < 2;
    }

private:
    /**
     * @brief Initialize the contour with a default ellipse.
     *
     * Used when no valid boundary is provided.
     */
    void defineFromEllipse();

    /**
     * @brief Remove redundant points from a boundary.
     *
     * Ensures the contour remains contiguous and minimal.
     */
    void eliminateRedundantPoints(ContourPoints& boundary, PhiValue regionValue);

    /**
     * @brief Allow ActiveContour to access internal helpers.
     */
    friend class ActiveContour;

    /**
     * @brief Check if a point is redundant in the boundary.
     *
     * A point is redundant if all its neighbors belong to the same region.
     */
    bool isRedundant(const ContourPoint& point) const;

    /**
     * @brief Allocate memory for boundary lists.
     */
    void allocateLists();

    /**
     * @brief Initialize phi and boundaries from a binary level-set.
     */
    void defineListsAndPhiFromBinaryPhi();

    /**
     * @brief Rebuild phi from boundary lists.
     */
    void definePhiFromLists();

    /**
     * @brief Flood fill used during phi reconstruction.
     */
    void floodFill(const Point2D_i& seed, PhiValue targetValue, PhiValue replacementValue);

    /**
     * @brief Remove redundant points in both boundaries if needed.
     */
    void eliminateRedundantPointsIfNeeded();

    /**
     * @brief Convert contour points to a geometric contour.
     */
    static Contour toContour(const ContourPoints& boundary);

    /**
     * @brief Check if a vector contains duplicate elements.
     */
    template <typename T>
    static bool hasDuplicates(const std::vector<T>& v);

    DiscreteLevelSet phi_; ///< Discrete level-set function (4 possible values).

    ContourPoints outerBoundary_; ///< Exterior boundary (Lout).
    ContourPoints innerBoundary_; ///< Interior boundary (Lin).

    const Connectivity connectivity_; ///< Pixel connectivity (4- or 8-connected).
};

/**
 * @brief Check if a vector contains duplicate elements.
 */
template <typename T>
bool ContourData::hasDuplicates(const std::vector<T>& v)
{
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        for (std::size_t j = i + 1; j < v.size(); ++j)
        {
            if (v[i] == v[j])
                return true;
        }
    }

    return false;
}

} // namespace fluvel_ip
