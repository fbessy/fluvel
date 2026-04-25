// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file shape_type.hpp
 * @brief Enumeration of supported geometric shapes.
 *
 * Defines the set of shapes that can be used for phi editing
 * and UI interactions.
 */

#pragma once

/**
 * @brief Supported shape types.
 *
 * Used to describe geometric primitives for editing operations
 * (e.g. adding or subtracting regions in the phi mask).
 */
enum class ShapeType
{
    Rectangle, ///< Axis-aligned rectangle
    Ellipse    ///< Elliptical shape
};
