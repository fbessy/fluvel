#ifndef POINT_HASH_HPP
#define POINT_HASH_HPP

#include "point.hpp"
#include <functional>

namespace std
{

template<>
struct hash<ofeli_ip::Point2D_i>
{
    size_t operator()(const ofeli_ip::Point2D_i& p) const noexcept
    {
        size_t h1 = std::hash<int>{}(p.x);
        size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

}

#endif // POINT_HASH_HPP
