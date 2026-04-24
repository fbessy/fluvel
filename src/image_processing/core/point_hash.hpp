// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point.hpp"
#include <functional>

namespace std
{

/**
 * @brief Hash specialization for fluvel_ip::Point2D_i.
 *
 * Enables the use of Point2D_i as a key in hash-based containers
 * such as std::unordered_set and std::unordered_map.
 *
 * The hash is computed by combining the hashes of the x and y coordinates.
 *
 * @param p Input point.
 * @return Combined hash value.
 *
 * @note The combination uses XOR and bit shifting:
 *       h = h1 ^ (h2 << 1)
 *
 * @warning This specialization must be defined in namespace std
 *          as required by the C++ standard for user-defined types.
 */
template <>
struct hash<fluvel_ip::Point2D_i>
{
    /**
     * @brief Compute hash of a 2D integer point.
     */
    size_t operator()(const fluvel_ip::Point2D_i& p) const noexcept
    {
        size_t h1 = std::hash<int>{}(p.x);
        size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

} // namespace std
