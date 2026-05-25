// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "boundary_builder.hpp"
#include "contour_data.hpp"
#include "neighborhood.hpp"

#include <cstddef>
#include <stack>

namespace fluvel_ip
{

ContourData::ContourData(int phiWidth, int phiHeight, Connectivity connectivity)
    : phi_(phiWidth, phiHeight)
    , connectivity_(connectivity)
{
    assert(phiWidth >= 1);
    assert(phiHeight >= 1);

    allocateLists();

    if (phiWidth < 2 || phiHeight < 2)
    {
        outerBoundary_.clear();
        innerBoundary_.clear();
        return;
    }

    defineFromEllipse();

    // post condition
    assert(isValid());
}

ContourData::ContourData(const ImageView& grayscalePhi, Connectivity connectivity)
    : phi_(grayscalePhi.width(), grayscalePhi.height())
    , connectivity_(connectivity)
{
    for (int y = 0; y < phi_.height(); ++y)
    {
        for (int x = 0; x < phi_.width(); ++x)
        {
            if (grayscalePhi.at(x, y) >= 128u)
                phi_.at(x, y) = PhiValue::InteriorBoundary;
            else
                phi_.at(x, y) = PhiValue::ExteriorBoundary;
        }
    }

    if (isTrivialDomain())
    {
        outerBoundary_.clear();
        innerBoundary_.clear();
        return;
    }

    defineListsAndPhiFromBinaryPhi();

    if (empty())
        defineFromEllipse();
    else
        eliminateRedundantPointsIfNeeded();

    // post condition
    assert(isValid());
}

ContourData::ContourData(const Contour& outerBoundary, const Contour& innerBoundary, int phiWidth,
                         int phiHeight, Connectivity connectivity)
    : phi_(phiWidth, phiHeight)
    , outerBoundary_(outerBoundary)
    , innerBoundary_(innerBoundary)
    , connectivity_(connectivity)
{
    assert(!outerBoundary.empty());
    assert(!innerBoundary.empty());
    assert(phiWidth >= 1);
    assert(phiHeight >= 1);

    allocateLists();

    if (isTrivialDomain())
    {
        outerBoundary_.clear();
        innerBoundary_.clear();
        return;
    }

    if (empty())
        defineFromEllipse();
    else
        definePhiFromLists();

    // post condition
    assert(isValid());
}

ContourData::ContourData(const ContourData& contour)
    : phi_(contour.phi_)
    , outerBoundary_(contour.outerBoundary_)
    , innerBoundary_(contour.innerBoundary_)
    , connectivity_(contour.connectivity_)
{
    allocateLists();
}

ContourData::ContourData(ContourData&& contour) noexcept
    : phi_(std::move(contour.phi_))
    , outerBoundary_(std::move(contour.outerBoundary_))
    , innerBoundary_(std::move(contour.innerBoundary_))
    , connectivity_(contour.connectivity_)
{
    allocateLists();
}

void ContourData::allocateLists()
{
    const size_t perimeter = static_cast<const size_t>(2 * (phi_.width() + phi_.height()));
    const size_t elem_alloc_size_ = 3 * perimeter;

    outerBoundary_.reserve(elem_alloc_size_);
    innerBoundary_.reserve(elem_alloc_size_);
}

void ContourData::defineFromEllipse()
{
    outerBoundary_.clear();
    innerBoundary_.clear();

    BoundaryBuilder lists_init(phi_.width(), phi_.height(), outerBoundary_, innerBoundary_);

    lists_init.generate_ellipse_points(0.8f, 0.8f);

    definePhiFromLists();
}

void ContourData::defineListsAndPhiFromBinaryPhi()
{
    for (size_t offset = 0; offset < phi_.size(); ++offset)
    {
        PhiValue& currentPhi = phi_[offset];
        PhiValue regionVal;
        Contour* boundary = nullptr;
        bool isBoundary = false;

        // get the generic currentMapping to eliminate redundant points
        if (currentPhi == PhiValue::ExteriorBoundary)
        {
            regionVal = PhiValue::OutsideRegion;
            boundary = &outerBoundary_;
            isBoundary = true;
        }
        else if (currentPhi == PhiValue::InteriorBoundary)
        {
            regionVal = PhiValue::InsideRegion;
            boundary = &innerBoundary_;
            isBoundary = true;
        }

        if (isBoundary)
        {
            const Point2D_i currentPoint = phi_.coord(offset);
            const ContourPoint point{currentPoint.x, currentPoint.y};

            if (isRedundant(point))
                currentPhi = regionVal;
            else
                boundary->push_back(point);
        }
    }
}

void ContourData::definePhiFromLists()
{
    phi_.fill(PhiValue::OutsideRegion);

    for (const auto& point : outerBoundary_)
        phi_.at(point.x(), point.y()) = PhiValue::ExteriorBoundary;

    for (const auto& point : innerBoundary_)
    {
        floodFill({point.x(), point.y()}, PhiValue::OutsideRegion, PhiValue::InsideRegion);

        phi_.at(point.x(), point.y()) = PhiValue::InteriorBoundary;
    }

    eliminateRedundantPointsIfNeeded();
}

void ContourData::floodFill(const Point2D_i& seed, PhiValue targetValue, PhiValue replacementValue)
{
    if (targetValue != replacementValue && phi_.valid(seed))
    {
        std::stack<Point2D_i> seedsStack;
        // top seed coordinates (x_ts,y_ts) and x for scan the row
        int x;
        bool spanUp, spanDown;

        seedsStack.push(seed);

        while (!seedsStack.empty())
        {
            // unstack the top seed
            const auto [x_ts, y_ts] = seedsStack.top();

            seedsStack.pop();

            // x initialization at the left-most point of the seed
            x = x_ts;
            while (x > 0 && phi_.at(x - 1, y_ts) == targetValue)
                x--;

            spanUp = false;
            spanDown = false;

            // pixels are treated row-wise
            while (x < phi_.width() && phi_.at(x, y_ts) == targetValue)
            {
                phi_.at(x, y_ts) = replacementValue;

                if (!spanUp && y_ts > 0 && phi_.at(x, y_ts - 1) == targetValue)
                {
                    seedsStack.emplace(x, y_ts - 1);
                    spanUp = true;
                }
                else if (spanUp && y_ts > 0 && phi_.at(x, y_ts - 1) != targetValue)
                {
                    spanUp = false;
                }

                if (!spanDown && y_ts < phi_.height() - 1 && phi_.at(x, y_ts + 1) == targetValue)
                {
                    seedsStack.emplace(x, y_ts + 1);
                    spanDown = true;
                }
                else if (spanDown && y_ts < phi_.height() - 1 &&
                         phi_.at(x, y_ts + 1) != targetValue)
                {
                    spanDown = false;
                }

                ++x;
            }
        }
    }
}

void ContourData::eliminateRedundantPointsIfNeeded()
{
    eliminateRedundantPoints(outerBoundary_, PhiValue::OutsideRegion);
    eliminateRedundantPoints(innerBoundary_, PhiValue::InsideRegion);
}

void ContourData::eliminateRedundantPoints(Contour& boundary, PhiValue regionValue)
{
    for (std::size_t i = 0; i < boundary.size();)
    {
        auto& point = boundary[i];

        if (isRedundant(point))
        {
            phi_.at(point.x(), point.y()) = regionValue;
            point = boundary.back();
            boundary.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

bool ContourData::isRedundant(const ContourPoint& point) const
{
    const int x = point.x();
    const int y = point.y();

    const int w = phi_.width();
    const int h = phi_.height();

    const auto phiCenter = phi_.at(x, y);
    const auto connectivity = connectivity_;

    // dn, neighbor position difference

    if (fullyInside8(x, y, w, h))
    {
        // FAST PATH: aucun valid(), aucune branche parasite

        for (const auto& dn : kNeighbors4)
        {
            if (phi_value::differentSide(phi_.at(x + dn.dx, y + dn.dy), phiCenter))
                return false;
        }

        if (connectivity == Connectivity::Eight)
        {
            for (const auto& dn : kNeighbors4Diag)
            {
                if (phi_value::differentSide(phi_.at(x + dn.dx, y + dn.dy), phiCenter))
                    return false;
            }
        }

        return true;
    }
    else
    {
        // SLOW PATH: bords

        for (const auto& dn : kNeighbors4)
        {
            const int nx = x + dn.dx;
            const int ny = y + dn.dy;

            if (!phi_.valid(nx, ny))
                continue;

            if (phi_value::differentSide(phi_.at(nx, ny), phiCenter))
                return false;
        }

        if (connectivity == Connectivity::Eight)
        {
            for (const auto& dn : kNeighbors4Diag)
            {
                const int nx = x + dn.dx;
                const int ny = y + dn.dy;

                if (!phi_.valid(nx, ny))
                    continue;

                if (phi_value::differentSide(phi_.at(nx, ny), phiCenter))
                    return false;
            }
        }

        return true;
    }
}

bool ContourData::isValid() const
{
    if (empty())
        return false;

    for (const auto& p : outerBoundary_)
    {
        if (phi_.at(p.x(), p.y()) != PhiValue::ExteriorBoundary)
            return false;

        if (isRedundant(p))
            return false;
    }

    for (const auto& p : innerBoundary_)
    {
        if (phi_.at(p.x(), p.y()) != PhiValue::InteriorBoundary)
            return false;

        if (isRedundant(p))
            return false;
    }

    Contour result;
    result.reserve(outerBoundary_.size() + innerBoundary_.size());
    result.insert(result.end(), outerBoundary_.begin(), outerBoundary_.end());
    result.insert(result.end(), innerBoundary_.begin(), innerBoundary_.end());

    if (hasDuplicates(result))
        return false;

    return true;
}

ExportedContour ContourData::exportContour(const Contour& boundary) const
{
    ExportedContour geometricBoundary;
    geometricBoundary.reserve(boundary.size());

    for (const auto& point : boundary)
    {
        geometricBoundary.emplace_back(point.x(), point.y());
    }

    return geometricBoundary;
}

} // namespace fluvel_ip
