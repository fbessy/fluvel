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

#include <cmath>    // std::exp
#include <cstring>  // std::memcpy

#ifndef IS_COLUMN_WISE
#define IS_COLUMN_WISE false
#endif

namespace ofeli_ip
{

template <typename T>
Matrix<T>::Matrix(T* matrix_data1, int width1, int height1) : matrix_data(matrix_data1),
    width(width1), height(height1), is_memory_ownership(false)
{
    if( matrix_data1 == nullptr )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, the pointer matrix_data1 must be a non-null pointer, it must be allocated.";
    }
    if( width1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, width1 must be strictly positive.";
    }
    if( height1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, height1 must be strictly positive.";
    }
}

template <typename T>
template <typename U>
Matrix<T>::Matrix(const U* matrix_data1, int width1, int height1) : matrix_data(new T[width1*height1]),
    width(width1), height(height1), is_memory_ownership(true)
{
    if( width1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, width1 must be strictly positive.";
    }
    if( height1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, height1 must be strictly positive.";
    }

    if( matrix_data1 != nullptr )
    {
        std::memcpy( matrix_data,
                     matrix_data1,
                     width*height*sizeof(T) );
    }
    else
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, the pointer must be a non-null pointer in order to copy data buffer.";
    }
}

template <typename T>
Matrix<T>::Matrix(int width1, int height1) : matrix_data(new T[width1*height1]),
    width(width1), height(height1), is_memory_ownership(true)
{
    if( width1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, width1 must be strictly positive.";
    }
    if( height1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, height1 must be strictly positive.";
    }
}

template <typename T>
Matrix<T>::Matrix(int width1, int height1, const T& value) : matrix_data(new T[width1*height1]),
    width(width1), height(height1), is_memory_ownership(true)
{
    if( width1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, width1 must be strictly positive.";
    }
    if( height1 <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, height1 must be strictly positive.";
    }

    memset(value);
}

template <typename T>
Matrix<T>::Matrix(const Matrix<T>& copied) : matrix_data(new T[copied.width*copied.height]),
    width( copied.width ), height( copied.height ), is_memory_ownership(true)
{
    if( width <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, width must be strictly positive.";
    }
    if( height <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, height must be strictly positive.";
    }

    if( copied.matrix_data != nullptr )
    {
        std::memcpy( matrix_data,
                     copied.matrix_data,
                     width*height*sizeof(T) );
    }
    else
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, the pointer encapsulated by the copied matrix must be a non-null pointer, it must be allocated.";
    }
}

template <typename T>
Matrix<T>::Matrix(Matrix<T>&& moved) noexcept : matrix_data(moved.matrix_data),
    width( moved.width ), height( moved.height ), is_memory_ownership( moved.is_memory_ownership )
{
    if ( &moved != this )
    {
        if( width <= 0 )
        {
            std::cerr << std::endl <<
                " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
                "Precondition, width must be strictly positive.";
        }
        if( height <= 0 )
        {
            std::cerr << std::endl <<
                " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
                "Precondition, height must be strictly positive.";
        }

        if( matrix_data != nullptr && is_memory_ownership )
        {
            moved.matrix_data = nullptr;
            // *this is now the ownership memory of the matrix data
            // so we set to nullptr the other pointer to avoid double memory freed.

            moved.is_memory_ownership = false;
        }
        else
        {
            std::cerr << std::endl <<
                " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
                "Precondition, the pointer encapsulated by the moved matrix must be a non-null pointer, it must be allocated.";
        }
    }
}

template <typename T>
int Matrix<T>::check_kernel_length(int value)
{
    if( value < 3 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, kernel_length1 must not be less than 3. It is set to 3.";

        value = 3;
    }
    else if( value % 2 == 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, kernel_length1 must be odd. It is decremented.";

        value--;
    }

    return value;
}

template <typename T>
Matrix<T>::Matrix(int kernel_length, float standard_deviation) :
    width( check_kernel_length(kernel_length) ),
    height( check_kernel_length(kernel_length) ),
    is_memory_ownership(true)
{
    matrix_data = new T[width*height];

    if( standard_deviation < 0.000000001f )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "Precondition, standard_deviation is positive and must not equal to zero. It is set to 0.000000001.";

        standard_deviation = 0.000000001f;
    }

    const int kernel_radius = (width- 1) / 2;

    for( int y = 0; y < height; y++ )
    {
        for( int x = 0; x < width; x++ )
        {
            (*this)(x,y) = T(  0.5f
                                 + 100000.f *
                                 std::exp( -( float( square(y-kernel_radius)+square(x-kernel_radius) ) )
                                           / (2.f*square(standard_deviation)) )
                                 );
        }
    }
}

template <typename T>
Matrix<T>& Matrix<T>::operator=(const Matrix<T>& rhs)
{
    if( this != &rhs ) // no auto-affectation
    {
        if( this->width == rhs.width &&
            this->height == rhs.height )
        {
            if( this->matrix_data != nullptr &&
                rhs.matrix_data != nullptr )
            {
                std::memcpy( this->matrix_data,
                             rhs.matrix_data,
                             width*height*sizeof(T) );
            }
            else
            {
                std::cerr << std::endl <<
                " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
                "Precondition, this->matrix_data or rhs.matrix_data must be a non-null pointer. It must be allocated.";
            }
        }
        else
        {
            std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "Precondition, buffers pointed by this->matrix_data and rhs.matrix_data need to have the same size.";
        }
    }
}

template <typename T>
Matrix<T>::~Matrix()
{
    if(    is_memory_ownership
        && matrix_data != nullptr )
    {
        delete[] matrix_data;
    }
}

template <typename T>
inline bool Matrix<T>::is_null() const
{
    return matrix_data == nullptr;
}

template <typename T>
void Matrix<T>::memset(const T& value)
{
    for( int offset = 0; offset < width*height; offset++ )
    {
        (*this)[offset] = value;
    }
}

template <typename T>
inline int Matrix<T>::get_offset(int x, int y) const
{
    if( IS_COLUMN_WISE )
    {
        return x*height+y;
    }
    else
    {
        return x+y*width;
    }
}

template <typename T>
inline int Matrix<T>::get_offset(const Point_i& point) const
{
    return get_offset( point.x, point.y );
}

template <typename T>
inline void Matrix<T>::get_position(int offset,
                                    int& x, int& y) const
{
    if( IS_COLUMN_WISE )
    {
        x = offset/height;
        y = offset-x*height;
    }
    else
    {
        y = offset/width;
        x = offset-y*width;
    }
}

template <typename T>
inline Point_i Matrix<T>::get_position(int offset) const
{
    int x, y;

    get_position( offset, x, y );

    return Point( x, y );
}

template <typename T>
inline const T& Matrix<T>::get_element(int offset) const
{
    return matrix_data[offset];
}

template <typename T>
inline const T& Matrix<T>::get_element(int x, int y) const
{
    return get_element( get_offset(x,y) );
}

template <typename T>
inline const T& Matrix<T>::get_element(const Point_i& point) const
{
    return get_element( get_offset( point ) );
}

template <typename T>
inline void Matrix<T>::set_element(int offset,
                                   const T& element)
{
    matrix_data[offset] = element;
}

template <typename T>
inline void Matrix<T>::set_element(int x, int y,
                                   const T& element)
{
    set_element( get_offset(x,y), element );
}

template <typename T>
inline void Matrix<T>::set_element(const Point_i& point,
                                   const T& element)
{
    set_element( get_offset(point), element );
}

template <typename U>
bool operator==(const Matrix<U>& lhs,
                const Matrix<U>& rhs)
{
    if( lhs.width == rhs.width &&
        lhs.height == rhs.height )
    {
        return !( std::memcmp( lhs.matrix_data,
                               rhs.matrix_data,
                               rhs.width*rhs.height*sizeof(U) )
                );
    }
    else
    {
        return false;
    }
}

template <typename U>
bool operator!=(const Matrix<U>& lhs,
                const Matrix<U>& rhs)
{
    return !( lhs == rhs );
}

template <typename T>
inline T& Matrix<T>::operator[](int offset)
{
    return matrix_data[offset];
}

template <typename T>
inline const T& Matrix<T>::operator[](int offset) const
{
    return matrix_data[offset];
}

template <typename T>
inline T& Matrix<T>::operator()(int x, int y)
{
    return matrix_data[ get_offset(x,y) ];
}

template <typename T>
inline const T& Matrix<T>::operator()(int x, int y) const
{
    return matrix_data[ get_offset(x,y) ];
}

template <typename U>
std::ostream& operator<<(std::ostream& os, const Matrix<U>& displayed)
{
    os << std::endl;
    for( int y = 0; y < displayed.get_height(); y++ )
    {
        for( int x = 0; x < displayed.get_width(); x++ )
        {
            os << "[" << x << "," << y << "] = " << displayed(x,y) << "   ";
        }
        os << std::endl;
    }

    return os;
}

template <typename T>
void Matrix<T>::display() const
{
    std::cout << *this;
}

}

