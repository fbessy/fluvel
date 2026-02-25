// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point.hpp"
#include <functional>

namespace std
{

template <> struct hash<ofeli_ip::Point2D_i>
{
    size_t operator()(const ofeli_ip::Point2D_i& p) const noexcept
    {
        size_t h1 = std::hash<int>{}(p.x);
        size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

} // namespace std
