// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace fluvel_ip
{

template <typename T> class Grid2D
{
public:
    Grid2D() = default;

    Grid2D(int width, int height)
        : width_(width)
        , height_(height)
        , data_(static_cast<size_t>(width * height))
    {
    }

    //! Grid2D methods

    int width() const noexcept
    {
        return width_;
    }
    int height() const noexcept
    {
        return height_;
    }

    T& at(int x, int y)
    {
        assert(valid(x, y));
        return data_[offset(x, y)];
    }

    const T& at(int x, int y) const
    {
        assert(valid(x, y));
        return data_[offset(x, y)];
    }

    //! Wrappers of std::vector<T>
    T& operator[](size_t offset) noexcept
    {
        return data_[offset];
    }
    const T& operator[](size_t offset) const noexcept
    {
        return data_[offset];
    }
    bool empty() const noexcept
    {
        return data_.empty();
    }
    size_t size() const noexcept
    {
        return data_.size();
    }
    T* data() noexcept
    {
        return data_.data();
    }
    const T* data() const noexcept
    {
        return data_.data();
    }

    //! Wrappers to allow loop for ( auto offset : phi_ )
    auto begin() noexcept
    {
        return data_.begin();
    }
    auto end() noexcept
    {
        return data_.end();
    }
    auto begin() const noexcept
    {
        return data_.begin();
    }
    auto end() const noexcept
    {
        return data_.end();
    }

    void fill(const T& value = T{})
    {
        std::fill(data_.begin(), data_.end(), value);
    }

    void clear()
    {
        data_.clear();
    }

    void resize(int width, int height)
    {
        width_ = width;
        height_ = height;

        data_.resize(width_ * height_);
    }

    Point2D_i coord(size_t offset) const noexcept
    {
        return {static_cast<int>(offset) % width_, static_cast<int>(offset) / width_};
    }

    size_t offset(int x, int y) const noexcept
    {
        return static_cast<size_t>(y * width_ + x);
    }

    bool valid(int x, int y) const noexcept
    {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    bool valid(const Point2D<int>& p) const noexcept
    {
        return valid(p.x, p.y);
    }

    bool valid(int offset) const noexcept
    {
        return offset >= 0 && offset < static_cast<int>(size());
    }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<T> data_;
};

} // namespace fluvel_ip
