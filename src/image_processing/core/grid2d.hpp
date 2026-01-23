/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <cassert>

#include "point.hpp"

namespace ofeli_ip
{

template<typename T>
class Grid2D
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

    int width()  const noexcept { return width_;  }
    int height() const noexcept { return height_; }

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
    T& operator[](size_t offset) noexcept { return data_[offset]; }
    const T& operator[](size_t offset) const noexcept { return data_[offset]; }
    bool empty() const noexcept { return data_.empty(); }
    size_t size() const noexcept { return data_.size(); }
    T* data() noexcept { return data_.data(); }
    const T* data() const noexcept { return data_.data(); }

    //! Wrappers to allow loop for ( auto offset : phi_ )
    auto begin() noexcept { return data_.begin(); }
    auto end()   noexcept { return data_.end(); }
    auto begin() const noexcept { return data_.begin(); }
    auto end()   const noexcept { return data_.end(); }

    void fill(const T& value)
    {
        std::fill(data_.begin(), data_.end(), value);
    }

    Point2D_i coord(size_t offset) const noexcept
    {
        return {
            static_cast<int>(offset % width_),
            static_cast<int>(offset / width_)
        };
    }

    size_t offset(int x, int y) const noexcept
    {
        return static_cast<size_t>(y * width_ + x);
    }

    bool valid(int x, int y) const noexcept
    {
        return x >= 0 && x < width_ &&
               y >= 0 && y < height_;
    }

    bool valid(const Point2D<int>& p) const noexcept
    {
        return valid( p.x, p.y );
    }

    bool valid(int offset) const noexcept
    {
        return offset >= 0 && offset < static_cast<int>( size() );
    }

private:
    int width_  = 0;
    int height_ = 0;
    std::vector<T> data_;
};

}

#endif // GRID_HPP
