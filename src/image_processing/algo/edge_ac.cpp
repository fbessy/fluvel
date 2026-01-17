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

#include "edge_ac.hpp"

#include <cstring>

namespace ofeli_ip
{

constexpr auto GRAYSCALE_DEPTH = 256u;

void EdgeAc::compute_external_speed_Fd(ContourPoint& point)
{
    const auto [x, y] = cd_.phi().coord( point.offset() );

    int max_out = 0;
    int max_in = 0;

    int dd;

    for( int dx = -1; dx <= 1; dx++ )
    {
        for( int dy = -1; dy <= 1; dy++ )
        {
            if(    x+dx >= 0 && x+dx < cd_.phi().width()
                && y+dy >= 0 && y+dy < cd_.phi().height() )
            {
                dd = std::abs( int( gradient_image.pixel_at(x,y) ) - int( gradient_image.pixel_at(x + dx, y + dy)  ) );

                if( phi_value::isOutside( cd_.phi().at(x + dx, y + dy) ) )
                {
                    max_out = std::max(max_out, dd);
                }
                else
                {
                    max_in = std::max(max_in, dd);
                }
            }
        }
    }

    int local_speed = max_out - max_in;

    int global_speed = global_speed_sign * (int(threshold) - int(gradient_image.pixel_at(x,y)));

    point.set_speed( get_discrete_speed( 3*local_speed + global_speed ) );
}

int EdgeAc::get_global_speed_sign() const
{
    int sign = 0;
    unsigned int sum_in = 0;
    unsigned int sum_out = 0;

    const int img_size = gradient_image.size();

    for( int offset = 0; offset < img_size; offset++ )
    {
        if( phi_value::isInside( cd_.phi()[offset] ) )
        {
            sum_in += (unsigned int)(gradient_image.pixel_at(offset));
        }
        else
        {
            sum_out += (unsigned int)(gradient_image.pixel_at(offset));
        }
    }

    if ( sum_out > sum_in )
    {
        sign = 1;
    }
    else
    {
        sign = -1;
    }

    return sign;
}

unsigned char EdgeAc::do_otsu_method(const ImageSpan8& image)
{
    unsigned char threshold = 0;

    unsigned int histogram[ GRAYSCALE_DEPTH ];

    std::memset( (void*)histogram, 0, GRAYSCALE_DEPTH * sizeof( unsigned int ) );

    for( int offset = 0;
         offset < image.size();
         offset++ )
    {
        histogram[ image.pixel_at(offset) ]++;
    }

    unsigned int sum = 0;
    for( unsigned int intensity = 0;
         intensity < GRAYSCALE_DEPTH;
         intensity++ )
    {
        sum += intensity * histogram[ intensity ];
    }

    unsigned int weight1, weight2, sum1;
    unsigned char mean1, mean2, t;
    float var_t, var_max;

    weight1 = 0;
    sum1 = 0;
    var_max = 0.f;

    // 256 values ==> 255 thresholds t evaluated
    // class1 <= t and class2 > t

    t = 0;
    weight2 = image.size();

    while( t < (GRAYSCALE_DEPTH-1) &&
           weight2 != 0 )
    {
        weight1 += histogram[t];
        weight2 = image.size()-weight1;

        if( weight1 != 0 &&
            weight2 != 0 )
        {
            sum1 += t*histogram[t];

            mean1 = (unsigned char) (sum1 / weight1);
            mean2 = (unsigned char) ( (sum-sum1) / weight2 ); // sum2 = sum-sum1

            var_t = (float)weight1 * (float)weight2 * (float)square( int(mean1)-int(mean2) );

            if( var_t > var_max )
            {
                var_max = var_t;
                threshold = t;
            }
        }

        t++;
    }

    return threshold;
}

}
