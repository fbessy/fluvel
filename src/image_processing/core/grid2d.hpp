// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace fluvel_ip
{

/**
 * @brief Generic 2D grid container stored in row-major order.
 *
 * Provides a lightweight abstraction over a contiguous 1D buffer,
 * with 2D indexing utilities and bounds checking (via assertions).
 *
 * @tparam T Element type.
 */
template <typename T>
class Grid2D
{
public:
    /// Default constructor.
    Grid2D() = default;

    /**
     * @brief Construct a grid with given dimensions.
     *
     * @param width Number of columns.
     * @param height Number of rows.
     */
    Grid2D(int width, int height)
        : width_(width)
        , height_(height)
        , data_(static_cast<size_t>(width * height))
    {
    }

    //! Grid2D methods

    /**
     * @brief Get grid width.
     */
    int width() const noexcept
    {
        return width_;
    }

    /**
     * @brief Get grid height.
     */
    int height() const noexcept
    {
        return height_;
    }

    /**
     * @brief Access element at (x, y) with bounds checking (assert).
     */
    T& at(int x, int y)
    {
        assert(valid(x, y));
        return data_[offset(x, y)];
    }

    /**
     * @brief Const access to element at (x, y) with bounds checking.
     */
    const T& at(int x, int y) const
    {
        assert(valid(x, y));
        return data_[offset(x, y)];
    }

    //! Wrappers of std::vector<T>

    /**
     * @brief Access element by linear offset.
     */
    T& operator[](size_t offset) noexcept
    {
        return data_[offset];
    }

    /**
     * @brief Const access by linear offset.
     */
    const T& operator[](size_t offset) const noexcept
    {
        return data_[offset];
    }

    /**
     * @brief Check if grid is empty.
     */
    bool empty() const noexcept
    {
        return data_.empty();
    }

    /**
     * @brief Get total number of elements.
     */
    size_t size() const noexcept
    {
        return data_.size();
    }

    /**
     * @brief Get raw data pointer.
     */
    T* data() noexcept
    {
        return data_.data();
    }

    /**
     * @brief Get const raw data pointer.
     */
    const T* data() const noexcept
    {
        return data_.data();
    }

    //! Iterators (range-based for support)

    /**
     * @brief Begin iterator.
     */
    auto begin() noexcept
    {
        return data_.begin();
    }

    /**
     * @brief End iterator.
     */
    auto end() noexcept
    {
        return data_.end();
    }

    /**
     * @brief Const begin iterator.
     */
    auto begin() const noexcept
    {
        return data_.begin();
    }

    /**
     * @brief Const end iterator.
     */
    auto end() const noexcept
    {
        return data_.end();
    }

    /**
     * @brief Fill grid with a value.
     *
     * @param value Value to assign to all elements.
     */
    void fill(const T& value = T{})
    {
        std::fill(data_.begin(), data_.end(), value);
    }

    /**
     * @brief Clear grid data (does not reset dimensions).
     */
    void clear()
    {
        data_.clear();
    }

    /**
     * @brief Resize grid.
     *
     * @param width New width.
     * @param height New height.
     */
    void resize(int width, int height)
    {
        width_ = width;
        height_ = height;

        data_.resize(width_ * height_);
    }

    /**
     * @brief Convert linear offset to 2D coordinates.
     *
     * @param offset Linear index.
     * @return Corresponding (x, y) coordinate.
     */
    Point2D_i coord(size_t offset) const noexcept
    {
        return {static_cast<int>(offset) % width_, static_cast<int>(offset) / width_};
    }

    /**
     * @brief Convert 2D coordinates to linear offset.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @return Linear index.
     */
    size_t offset(int x, int y) const noexcept
    {
        return static_cast<size_t>(y * width_ + x);
    }

    /**
     * @brief Check if coordinates are inside the grid.
     */
    bool valid(int x, int y) const noexcept
    {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    /**
     * @brief Check if a point is inside the grid.
     */
    bool valid(const Point2D<int>& p) const noexcept
    {
        return valid(p.x, p.y);
    }

    /**
     * @brief Check if a linear offset is valid.
     */
    bool valid(int offset) const noexcept
    {
        return offset >= 0 && offset < static_cast<int>(size());
    }

    /**
     * @brief Get pointer to the beginning of a row.
     *
     * @param y Row index.
     * @return Pointer to row data.
     */
    const T* row(int y) const
    {
        return data() + y * width_;
    }

private:
    /// Grid width.
    int width_{0};

    /// Grid height.
    int height_{0};

    /// Contiguous storage (row-major).
    std::vector<T> data_;
};

} // namespace fluvel_ip
