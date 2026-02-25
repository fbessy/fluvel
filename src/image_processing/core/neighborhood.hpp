#ifndef NEIGHBORHOOD_HPP
#define NEIGHBORHOOD_HPP

namespace ofeli_ip
{

struct Vec2i
{
    int dx;
    int dy;
};

// 4-connexité : voisins cardinaux (faces)
constexpr Vec2i kNeighbors4[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

// Complément diagonaux (pour passer à 8-connexité)
constexpr Vec2i kNeighbors4Diag[] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

inline bool fully_inside_8(int x, int y, int w, int h)
{
    return x > 0 && x + 1 < w && y > 0 && y + 1 < h;
}

} // namespace ofeli_ip

#endif // NEIGHBORHOOD_HPP
