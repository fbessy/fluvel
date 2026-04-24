// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_ip
{

/**
 * @brief 2D integer vector representing a relative offset.
 *
 * This structure is mainly used to encode neighbor displacements
 * in a discrete grid (e.g., for connectivity traversal or kernel iteration).
 */
struct Vec2i
{
    int dx; ///< Horizontal offset (x-axis)
    int dy; ///< Vertical offset (y-axis)
};

/**
 * @brief 4-connectivity neighbors (cardinal directions).
 *
 * Defines offsets for face-adjacent pixels in a 2D grid:
 * - left  (-1,  0)
 * - right ( 1,  0)
 * - up    ( 0, -1)
 * - down  ( 0,  1)
 *
 * Commonly used in:
 * - region growing
 * - contour evolution (Lin / Lout)
 * - morphological operations
 */
constexpr Vec2i kNeighbors4[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

/**
 * @brief Diagonal neighbors complement for 8-connectivity.
 *
 * Provides the diagonal offsets required to extend 4-connectivity
 * to full 8-connectivity:
 * - top-left     (-1, -1)
 * - top-right    ( 1, -1)
 * - bottom-left  (-1,  1)
 * - bottom-right ( 1,  1)
 *
 * Typically used together with kNeighbors4.
 */
constexpr Vec2i kNeighbors4Diag[] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

/**
 * @brief Check if a pixel is strictly inside the image domain (8-connectivity safe zone).
 *
 * A pixel is considered "fully inside" if all its 8-connected neighbors
 * are guaranteed to lie within image bounds.
 *
 * This allows skipping boundary checks when accessing neighbors,
 * which is useful for performance-critical inner loops.
 *
 * @param x Pixel x-coordinate.
 * @param y Pixel y-coordinate.
 * @param w Image width.
 * @param h Image height.
 *
 * @return true if all 8 neighbors are valid, false otherwise.
 */
inline bool fullyInside8(int x, int y, int w, int h)
{
    return x > 0 && x + 1 < w && y > 0 && y + 1 < h;
}

} // namespace fluvel_ip
