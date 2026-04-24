// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point_hash.hpp"
#include <unordered_set>

namespace fluvel_ip
{

/**
 * @brief Hash-based set of 2D integer points.
 *
 * This type is an alias for std::unordered_set<Point2D_i> using a custom hash
 * (defined in point_hash.hpp).
 *
 * It provides:
 * - fast insertion and lookup (average O(1))
 * - uniqueness of points
 *
 * Typical use cases:
 * - intersection between shapes
 * - duplicate detection
 * - spatial membership queries
 *
 * @note Requires a valid hash function for Point2D_i.
 */
using PointSet = std::unordered_set<Point2D_i>;

} // namespace fluvel_ip
