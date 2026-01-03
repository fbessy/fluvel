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

#include "windows_tools.hpp"
#include "contour_data.hpp"

namespace ofeli_gui
{

void get_color(int index,
               RgbColor& color)
{
    switch( index )
    {
    case QComboBoxColorIndex::RED :
        color.red   = 255;
        color.green = 0;
        color.blue  = 0;
        break;

    case QComboBoxColorIndex::GREEN :
        color.red   = 0;
        color.green = 255;
        color.blue  = 0;
        break;

    case QComboBoxColorIndex::BLUE :
        color.red   = 0;
        color.green = 0;
        color.blue  = 255;
        break;

    case QComboBoxColorIndex::CYAN :
        color.red   = 0;
        color.green = 255;
        color.blue  = 255;
        break;

    case QComboBoxColorIndex::MAGENTA :
        color.red   = 255;
        color.green = 0;
        color.blue  = 255;
        break;

    case QComboBoxColorIndex::YELLOW :
        color.red   = 255;
        color.green = 255;
        color.blue  = 0;
        break;

    case QComboBoxColorIndex::BLACK :
    case QComboBoxColorIndex::NO :
    default :
        color.red   = 0;
        color.green = 0;
        color.blue  = 0;
        break;

    case QComboBoxColorIndex::WHITE :
        color.red   = 255;
        color.green = 255;
        color.blue  = 255;
        break;
    }
}

void draw_list_to_img(const std::vector<ofeli_ip::ContourPoint>& list,
                      const RgbColor& color,
                      int combobox_index,
                      unsigned char* img_rgb32_data,
                      int img_rgb32_width,
                      int img_rgb32_height)
{
    assert(img_rgb32_data != nullptr
           && img_rgb32_width > 0
           && img_rgb32_height >0 );

    if( combobox_index != QComboBoxColorIndex::NO )
    {
        int offset;

        for( const auto& point : list )
        {
            offset = point.get_offset();

            assert(offset >= 0 && offset < img_rgb32_width*img_rgb32_height );

            offset *= 4;

            img_rgb32_data[ offset+2 ] = color.red;
            img_rgb32_data[ offset+1 ] = color.green;
            img_rgb32_data[ offset   ] = color.blue;
        }
    }
}

void draw_upscale_list(const std::vector<ofeli_ip::ContourPoint>& list,
                       const RgbColor& color,
                       int combobox_index,
                       unsigned int upscale_factor,
                       QImage& img_rgb32)
{
    if (    upscale_factor == 2
         || upscale_factor == 4 )
    {
        if ( combobox_index != QComboBoxColorIndex::NO )
        {
            int offset;
            int x, y;
            unsigned int small_img_width = img_rgb32.width() / upscale_factor;

            const int kernel_radius = upscale_factor / 2;

            unsigned char* img_rgb32_data = img_rgb32.bits();

            for( const auto& point : list )
            {
                offset = point.get_offset();

                y = offset/small_img_width;
                x = offset-y*small_img_width;

                if (    (int)upscale_factor*x+kernel_radius < img_rgb32.width()
                     && (int)upscale_factor*y+kernel_radius < img_rgb32.height()
                     && (int)upscale_factor*x-kernel_radius+1 >= 0
                     && (int)upscale_factor*y-kernel_radius+1 >= 0 )
                {
                    for( int dx = -kernel_radius+1; dx <= kernel_radius; dx++ )
                    {
                        for( int dy = -kernel_radius+1; dy <= kernel_radius; dy++ )
                        {
                            offset = (upscale_factor*x+dx)+(upscale_factor*y+dy)*img_rgb32.width();

                            assert(offset >= 0 && offset < img_rgb32.width()*img_rgb32.height() );

                            img_rgb32_data[ 4*offset+2 ] = color.red;
                            img_rgb32_data[ 4*offset+1 ] = color.green;
                            img_rgb32_data[ 4*offset   ] = color.blue;
                        }
                    }
                }
                else
                {
                    for( int dx = -kernel_radius+1; dx <= kernel_radius; dx++ )
                    {
                        for( int dy = -kernel_radius+1; dy <= kernel_radius; dy++ )
                        {
                            if (    (int)upscale_factor*x+dx < img_rgb32.width()
                                 && (int)upscale_factor*y+dy < img_rgb32.height()
                                 && (int)upscale_factor*x+dx >= 0
                                 && (int)upscale_factor*y+dy >= 0 )
                            {
                                offset = (upscale_factor*x+dx)+(upscale_factor*y+dy)*img_rgb32.width();

                                assert(offset >= 0 && offset < img_rgb32.width()*img_rgb32.height() );

                                img_rgb32_data[ 4*offset+2 ] = color.red;
                                img_rgb32_data[ 4*offset+1 ] = color.green;
                                img_rgb32_data[ 4*offset   ] = color.blue;
                            }
                        }
                    }
                }
            }
        }
    }
}

void draw_list_to_img1_img2(const std::vector<ofeli_ip::ContourPoint>& list,
                            const RgbColor& color1,
                            const RgbColor& color2,
                            unsigned char* img1_rgb_data,
                            unsigned char* img2_rgb_data)
{
    int offset;

    for( const auto& point : list )
    {
        offset = point.get_offset() * 4;

        img1_rgb_data[ offset+2 ] = color1.red;
        img1_rgb_data[ offset+1 ] = color1.green;
        img1_rgb_data[ offset   ] = color1.blue;

        img2_rgb_data[ offset+2 ] = color2.red;
        img2_rgb_data[ offset+1 ] = color2.green;
        img2_rgb_data[ offset   ] = color2.blue;
    }
}

void erase_list_to_img(const std::vector<ofeli_ip::ContourPoint>& list,
                       const unsigned char* img_to_copy,
                       unsigned char* img_rgb_data)
{
    int offset;

    for( const auto& point : list )
    {
        offset = point.get_offset() * 4;

        img_rgb_data[ offset+2 ] = img_to_copy[ offset+2 ];
        img_rgb_data[ offset+1 ] = img_to_copy[ offset+1 ];
        img_rgb_data[ offset   ] = img_to_copy[ offset   ];
    }
}

void erase_list_to_img1_img2(const std::vector<ofeli_ip::ContourPoint>& list,
                             const unsigned char* img_to_copy,
                             unsigned char* img1_rgb_data,
                             unsigned char* img2_rgb_data)
{
    int offset;

    for( const auto& point : list )
    {
        offset = point.get_offset() * 4;

        img1_rgb_data[ offset+2 ] = img_to_copy[ offset+2 ];
        img1_rgb_data[ offset+1 ] = img_to_copy[ offset+1 ];
        img1_rgb_data[ offset   ] = img_to_copy[ offset   ];

        img2_rgb_data[ offset+2 ] = img_to_copy[ offset+2 ];
        img2_rgb_data[ offset+1 ] = img_to_copy[ offset+1 ];
        img2_rgb_data[ offset   ] = img_to_copy[ offset   ];
    }
}

void erase_list_to_img_grayscale(const std::vector<ofeli_ip::ContourPoint>& list,
                                 const unsigned char* img_grayscale_to_copy,
                                 unsigned char* img_rgb_data)
{
    int offset;
    unsigned char intensity;

    for( const auto& point : list )
    {
        offset = point.get_offset();
        intensity = img_grayscale_to_copy[ offset ];

        img_rgb_data[ 4*offset+2 ] = intensity;
        img_rgb_data[ 4*offset+1 ] = intensity;
        img_rgb_data[ 4*offset   ] = intensity;
    }
}

}
