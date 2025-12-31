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

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream> // operator<< overloading

#include "point.hpp"

namespace ofeli_ip
{

template <typename T>
class Matrix
{

public :

    //! Constructor.
    explicit Matrix(T* matrix_data1, int width1, int height1);

    //! Constructor.
    template <typename U>
    explicit Matrix(const U* matrix_data1, int width1, int height1);

    //! Constructor.
    explicit Matrix(int width1, int height1);

    //! Constructor.
    explicit Matrix(int width1, int height1, const T& value);

    //! Creates a normalised gaussian kernel without to divide by \a π.
    explicit Matrix(int kernel_length, float standard_deviation);

    //! Copy constructor.
    Matrix(const Matrix& copied);

    //! Move constructor.
    Matrix(Matrix&& moved) noexcept;

    //! Destructor.
    ~Matrix();

    //! Assignment operator overloading.
    Matrix& operator=(const Matrix& rhs);

    //! Returns if the encapsulated pointer is allocated or not.
    bool is_null() const;

    //! Assign each value of the internal buffer by the input value.
    void memset(const T& value);

    //! Gets offset for a given position (x,y).
    int get_offset(int x, int y) const;
    //! Gets offset for a given position.
    int get_offset(const Point_i& point) const;

    //! Gets position (x,y) for a given offset.
    void get_position(int offset,
                      int& x, int& y) const;
    //! Gets position for a given offset.
    Point_i get_position(int offset) const;

    //! Gets the element for a given offset.
    const T& get_element(int offset) const;
    //! Gets the element for a given position (x,y).
    const T& get_element(int x, int y) const;
    const T& get_element(const Point_i& point) const;

    //! Sets the element at the given offset.
    void set_element(int offset, const T& element);
    //! Sets the element at the given position (x,y).
    void set_element(int x, int y, const T& element);
    //! Sets the element at the given position.
    void set_element(const Point_i& point, const T& element);

    //! \a Equal \a to operator overloading.
    template <typename U>
    friend bool operator==(const Matrix<U>& lhs, const Matrix<U>& rhs);

    //! \a Not \a equal \a to operator overloading.
    template <typename U>
    friend bool operator!=(const Matrix<U>& lhs, const Matrix<U>& rhs);

    T& operator[](int offset);
    const T& operator[](int offset) const;
    T& operator()(int x, int y);
    const T& operator()(int x, int y) const;

    //! Overloading of cout <<. It displays a linked list in the same way as integral-type variable.
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const Matrix<U>& displayed);

    //! A second way to display a linked list.
    void display() const;

    const T* get_matrix_data() const { return matrix_data; }
    int get_width() const { return width; }
    int get_height() const { return height; }

private :

    //! Matrix data.
    T* matrix_data;

    //! Width of the matrix, i.e. number of columns.
    const int width;

    //! Height of the matrix, i.e. number of rows.
    const int height;

    bool is_memory_ownership;

    //! Gives the square of a value.
    template <typename U>
    static U square(const U& value);

    static int check_kernel_length(int value);
};

template <typename T>
template <typename U>
inline U Matrix<T>::square(const U& value)
{
    return value*value;
}

}

// list definitions
#include "matrix.txx"

#endif // MATRIX_HPP
