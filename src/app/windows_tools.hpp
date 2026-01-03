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

#ifndef WINDOWS_TOOLS_HPP
#define WINDOWS_TOOLS_HPP

#include "contour_data.hpp"
#include <QRgb>
#include <QImage>

namespace ofeli_gui
{

struct RgbColor
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;

    QRgb get_QRgb() { return qRgb(int(red),
                                  int(green),
                                  int(blue)); }

    RgbColor divide(unsigned char n)
    {
        RgbColor tmp;

        tmp.red = red / n;
        tmp.green = green / n;
        tmp.blue = blue / n;

        return tmp;
    }
};

enum QComboBoxColorIndex : int
{
    RED = 0,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    YELLOW,
    BLACK,
    WHITE,
    SELECTED,
    NO
};

void get_color(int index,
               RgbColor& color);

void draw_list_to_img(const std::vector<ofeli_ip::ContourPoint>& list,
                      const RgbColor& color,
                      int combobox_index,
                      unsigned char* img_rgb32_data,
                      int img_rgb32_width,
                      int img_rgb32_height);

void draw_upscale_list(const std::vector<ofeli_ip::ContourPoint>& list,
                       const RgbColor& color,
                       int combobox_index,
                       unsigned int upscale_factor,
                       QImage& img_rgb32);

void draw_list_to_img1_img2(const std::vector<ofeli_ip::ContourPoint>& list,
                            const RgbColor& color1,
                            const RgbColor& color2,
                            unsigned char* img1_rgb_data,
                            unsigned char* img2_rgb_data);

void erase_list_to_img(const std::vector<ofeli_ip::ContourPoint>& list,
                       const unsigned char* img_to_copy,
                       unsigned char* img_rgb_data);

void erase_list_to_img1_img2(const std::vector<ofeli_ip::ContourPoint>& list,
                             const unsigned char* img_to_copy,
                             unsigned char* img1_rgb_data,
                             unsigned char* img2_rgb_data);

void erase_list_to_img_grayscale(const std::vector<ofeli_ip::ContourPoint>& list,
                                 const unsigned char* img_grayscale_to_copy,
                                 unsigned char* img_rgb_data);

}

#endif // WINDOWS_TOOLS_HPP
