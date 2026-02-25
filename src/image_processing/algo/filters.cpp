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

#include "filters.hpp"

#include <algorithm>
#include <chrono> // to generate a seed for the random generator
#include <cstddef>
#include <functional> // for function "std::bind" to link a generator and a distribution
#include <iostream>   // std::cerr
#include <random>     // random generator and normal/uniform distributions

#include <cmath>   // std::exp
#include <cstring> // std::memcpy

#include "ofeli_math.hpp"

namespace ofeli_ip
{

Filters::Filters(const unsigned char* img_data1, int img_width1, int img_height1,
                 int byte_per_pixel1)
    : img_data_(img_data1)
    , img_width_(img_width1)
    , img_height_(img_height1)
    , img_size_(img_width1 * img_height1)
    , byte_per_pixel_(byte_per_pixel1)
    , filtered_(
          new unsigned char[static_cast<unsigned long>(byte_per_pixel1 * img_width1 * img_height1)])
    , filtered_modif_(
          new unsigned char[static_cast<unsigned long>(byte_per_pixel1 * img_width1 * img_height1)])
    , previous_filtered_(
          new unsigned char[static_cast<unsigned long>(byte_per_pixel1 * img_width1 * img_height1)])
    , diff_img_(new float[static_cast<unsigned long>(img_width1 * img_height1)])
    , diff_img1_(new float[static_cast<unsigned long>(img_width1 * img_height1)])
    , columns_histo_(new int[static_cast<unsigned long>(256 * img_width1)])
{
    if (img_data1 == nullptr)
    {
        std::cerr << '\n'
                  << " ==> " << __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << '\n'
                  << "The pointer img_data1 must be a non-null pointer, it must be allocated.";
    }

    if (byte_per_pixel_ == 3)
    {
        gradient_ = new unsigned char[img_size_];
    }

    initialyze_filtered();
}

void Filters::initialyze_filtered()
{
    std::memcpy(filtered_, img_data_, static_cast<size_t>(byte_per_pixel_ * img_size_));
}

Filters::~Filters()
{
    // buffer deleted for the Perreault's algorithm
    delete[] columns_histo_;

    // buffers deleted for the anisotropic filter
    delete[] diff_img1_;
    delete[] diff_img_;

    if (byte_per_pixel_ == 3)
    {
        delete[] gradient_;
    }

    delete[] previous_filtered_;
    delete[] filtered_modif_;
    delete[] filtered_;
}

void Filters::anisotropic_diffusion(int max_itera, float lambda, float kappa, AnisoDiff option)
{
    if (kappa < 0.000000001f)
    {
        kappa = 0.000000001f;
    }

    // euclidian distance square of the neighbor pixels to the center
    // const float dist_sqr[8] = { dd*dd, dy*dy, dd*dd, dx*dx, dx*dx, dd*dd, dy*dy, dd*dd };
    const float dist_sqr[8] = {2.f, 1.f, 2.f, 1.f, 1.f, 2.f, 1.f, 2.f};

    // index for 0 to 7 to access of the 8 gradients and the 8 diffusion coefficients
    int index;
    // sum calculated on the 8 directions for the PDE solving
    float sigma;

    // 8 gradients for each direction for the current pixel n
    float nabla[8];

    // intensity of the current central pixel
    float centrI;

    // coefficient of diffusion for each direction
    float c;

    // temporary pointer to swap the pointers of diffused images
    float* ptemp_aniso;

    float result_f;

    int x, y; // position of the current pixel

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        // initial condition of the PDE
        for (int offset = 0; offset < img_size_; offset++)
        {
            diff_img_[offset] = float(filtered_[byte_per_pixel_ * offset + color_channel]);
        }

        // choice of the function of diffusion
        switch (option)
        {
            // c(x,y,t) = exp(-(nabla/kappa)^2)
            case AnisoDiff::FUNCTION1:
            {
                // anisotropic diffusion
                for (int itera = 0; itera < max_itera; itera++)
                {
                    // finite differences
                    for (int offset = 0; offset < img_size_; offset++)
                    {
                        y = offset / img_width_;
                        x = offset - y * img_width_;

                        centrI = diff_img_[offset];

                        index = 0;

                        // if the current pixel is not in the border
                        if (x > 0 && x < img_width_ - 1 && y > 0 && y < img_height_ - 1)
                        {
                            // no tests of neighbors
                            for (int dy = -1; dy <= 1; dy++)
                            {
                                for (int dx = -1; dx <= 1; dx++)
                                {
                                    if (!(dx == 0 && dy == 0))
                                    {
                                        nabla[index++] =
                                            diff_img_[(x + dx) + (y + dy) * img_width_] - centrI;
                                    }
                                }
                            }
                        }

                        else
                        {
                            for (int dy = -1; dy <= 1; dy++)
                            {
                                for (int dx = -1; dx <= 1; dx++)
                                {
                                    if (!(dx == 0 && dy == 0))
                                    {
                                        if (x + dx >= 0 && x + dx < img_width_ && y + dy >= 0 &&
                                            y + dy < img_height_)
                                        {
                                            nabla[index++] =
                                                diff_img_[(x + dx) + (y + dy) * img_width_] -
                                                centrI;
                                        }
                                        else
                                        {
                                            nabla[index++] = 0.f;
                                        }
                                    }
                                }
                            }
                        }

                        sigma = 0.f;
                        for (index = 0; index < 8; index++)
                        {
                            c = std::exp(-math::square(nabla[index] / kappa));

                            sigma += (1.f / (dist_sqr[index])) * c * nabla[index];
                        }

                        // solution of the discret PDE
                        diff_img1_[offset] = centrI + lambda * sigma;
                    }

                    // swap pointers
                    ptemp_aniso = diff_img_;
                    diff_img_ = diff_img1_;
                    diff_img1_ = ptemp_aniso;
                }

                break;
            }

            // c(x,y,t) = 1/(1+(1+(nabla/kappa)^2)
            case AnisoDiff::FUNCTION2:
            {
                // anisotropic diffusion
                for (int itera = 0; itera < max_itera; itera++)
                {
                    // finite differences
                    for (int offset = 0; offset < img_size_; offset++)
                    {
                        y = offset / img_width_;
                        x = offset - y * img_width_;

                        centrI = diff_img_[offset];

                        // if the current pixel is not in the border
                        if (x > 0 && x < img_width_ - 1 && y > 0 && y < img_height_ - 1)
                        {
                            // no tests of neighbors
                            index = 0;
                            for (int dy = -1; dy <= 1; dy++)
                            {
                                for (int dx = -1; dx <= 1; dx++)
                                {
                                    if (!(dx == 0 && dy == 0))
                                    {
                                        nabla[index++] =
                                            diff_img_[(x + dx) + (y + dy) * img_width_] - centrI;
                                    }
                                }
                            }
                        }

                        else
                        {
                            index = 0;
                            for (int dy = -1; dy <= 1; dy++)
                            {
                                for (int dx = -1; dx <= 1; dx++)
                                {
                                    if (!(dx == 0 && dy == 0))
                                    {
                                        if (x + dx >= 0 && x + dx < img_width_ && y + dy >= 0 &&
                                            y + dy < img_height_)
                                        {
                                            nabla[index++] =
                                                diff_img_[(x + dx) + (y + dy) * img_width_] -
                                                centrI;
                                        }
                                        else
                                        {
                                            nabla[index++] = 0.f;
                                        }
                                    }
                                }
                            }
                        }

                        sigma = 0.0;
                        for (index = 0; index < 8; index++)
                        {
                            c = 1.f / (1.f + math::square(nabla[index] / kappa));

                            sigma += (1.f / (dist_sqr[index])) * c * nabla[index];
                        }

                        // solution of the discret PDE
                        diff_img1_[offset] = centrI + lambda * sigma;
                    }

                    // swap pointers
                    ptemp_aniso = diff_img_;
                    diff_img_ = diff_img1_;
                    diff_img1_ = ptemp_aniso;
                }
            }
        }

        // result
        for (int offset = 0; offset < img_size_; offset++)
        {
            result_f = roundf(diff_img_[offset]);

            if (result_f > 255.f)
            {
                result_f = 255.f;
            }
            else if (result_f < 0.f)
            {
                result_f = 0.f;
            }

            filtered_[byte_per_pixel_ * offset + color_channel] = (unsigned char)(result_f);
        }
    }
}

void Filters::morphological_gradient(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length - 1) / 2;

    int x, y; // position of the current pixel

    unsigned char max, min;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            // initialization
            max = 0;
            min = 255;

            // if the current pixel is not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // no tests of neighbors
                        if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                      color_channel] > max)
                        {
                            max = filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                            color_channel];
                        }
                        if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                      color_channel] < min)
                        {
                            min = filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                            color_channel];
                        }
                    }
                }
            }
            // if in the border
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // neighbors tests
                        if (x + dx >= 0 && x + dx < img_width_ && y + dy >= 0 &&
                            y + dy < img_height_)
                        {
                            if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel] > max)
                            {
                                max =
                                    filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                              color_channel];
                            }
                            if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel] < min)
                            {
                                min =
                                    filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                              color_channel];
                            }
                        }
                    }
                }
            }
            filtered_modif_[byte_per_pixel_ * offset + color_channel] = max - min;
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

void Filters::morphological_gradient_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length - 1) / 2;

    int I;
    unsigned char max, min;

    int x, y; // position of the current pixel

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = 0; y < kernel_radius + 1; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius + 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the maximum value in the kernel histogram
                    for (max = 255; kernel_histo_[max] == 0; max--)
                    {
                    }

                    // to find the minimum value in the kernel histogram
                    for (min = 0; kernel_histo_[min] == 0; min++)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = max - min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius + 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max - min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max - min;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = kernel_radius + 1; y <= img_height_ - kernel_radius - 2; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the maximum value in the kernel histogram
                    for (max = 255; kernel_histo_[max] == 0; max--)
                    {
                    }

                    // to find the minimum value in the kernel histogram
                    for (min = 0; kernel_histo_[min] == 0; min++)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = max - min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max - min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max - min;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for (y = img_height_ - kernel_radius - 1; y < img_height_; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the maximum value in the kernel histogram
                    for (max = 255; kernel_histo_[max] == 0; max--)
                    {
                    }

                    // to find the minimum value in the kernel histogram
                    for (min = 0; kernel_histo_[min] == 0; min++)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = max - min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max - min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max - min;
            }
        }
    }

    if (kernel_length != 1)
    {
        // swap pointers
        unsigned char* ptemp = filtered_;
        filtered_ = filtered_modif_;
        filtered_modif_ = ptemp;
    }
}

void Filters::dilation(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length - 1) / 2;

    int x, y; // position of the current pixel

    unsigned char max;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            // initialisation
            max = 0;

            // if the current pixel is not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // no neighbors tests
                        if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                      color_channel] > max)
                        {
                            max = filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                            color_channel];
                        }
                    }
                }
            }
            // if in the border
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // neighbors tests
                        if (x + dx >= 0 && x + dx < img_width_ && y + dy >= 0 &&
                            y + dy < img_height_)
                        {
                            if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel] > max)
                            {
                                max =
                                    filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                              color_channel];
                            }
                        }
                    }
                }
            }
            filtered_modif_[byte_per_pixel_ * offset + color_channel] = max;
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

void Filters::dilation_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length - 1) / 2;

    int I;
    unsigned char max;

    int x, y; // position of the current pixel

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = 0; y < kernel_radius + 1; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius + 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the maximum value in the kernel histogram
                    for (max = 255; kernel_histo_[max] == 0; max--)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = max;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius + 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = kernel_radius + 1; y <= img_height_ - kernel_radius - 2; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the maximum value in the kernel histogram
                    for (max = 255; kernel_histo_[max] == 0; max--)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = max;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for (y = img_height_ - kernel_radius - 1; y < img_height_; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the maximum value in the kernel histogram
                    for (max = 255; kernel_histo_[max] == 0; max--)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = max;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the maximum value in the kernel histogram
                for (max = 255; kernel_histo_[max] == 0; max--)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = max;
            }
        }
    }

    if (kernel_length != 1)
    {
        // swap pointers
        unsigned char* ptemp = filtered_;
        filtered_ = filtered_modif_;
        filtered_modif_ = ptemp;
    }
}

void Filters::erosion(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length - 1) / 2;

    int x, y; // position of the current pixel

    unsigned char min;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            // initialization
            min = 255;

            // if the current pixel is not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // no neighbors tests
                        if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                      color_channel] < min)
                        {
                            min = filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                            color_channel];
                        }
                    }
                }
            }
            // if in the border
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // neighbors tests
                        if (x + dx >= 0 && x + dx < img_width_ && y + dy >= 0 &&
                            y + dy < img_height_)
                        {
                            if (filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel] < min)
                            {
                                min =
                                    filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                              color_channel];
                            }
                        }
                    }
                }
            }
            filtered_modif_[byte_per_pixel_ * offset + color_channel] = min;
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

void Filters::erosion_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length - 1) / 2;

    int I;
    unsigned char min;

    int x, y; // position of the current pixel

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = 0; y < kernel_radius + 1; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius + 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the minimum value in the kernel histogram
                    for (min = 0; kernel_histo_[min] == 0; min++)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius + 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = min;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = kernel_radius + 1; y <= img_height_ - kernel_radius - 2; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the minimum value in the kernel histogram
                    for (min = 0; kernel_histo_[min] == 0; min++)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = min;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for (y = img_height_ - kernel_radius - 1; y < img_height_; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the minimum value in the kernel histogram
                    for (min = 0; kernel_histo_[min] == 0; min++)
                    {
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] = min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the minimum value in the kernel histogram
                for (min = 0; kernel_histo_[min] == 0; min++)
                {
                }

                filtered_modif_[find_offset(x, y, color_channel)] = min;
            }
        }
    }

    if (kernel_length != 1)
    {
        // swap pointers
        unsigned char* ptemp = filtered_;
        filtered_ = filtered_modif_;
        filtered_modif_ = ptemp;
    }
}

void Filters::closing(int kernel_length)
{
    dilation(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif_;
    filtered_modif_ = previous_filtered_;
    previous_filtered_ = ptemp;

    erosion(kernel_length);
}

void Filters::closing_o1(int kernel_length)
{
    dilation_o1(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif_;
    filtered_modif_ = previous_filtered_;
    previous_filtered_ = ptemp;

    erosion_o1(kernel_length);
}

void Filters::opening(int kernel_length)
{
    erosion(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif_;
    filtered_modif_ = previous_filtered_;
    previous_filtered_ = ptemp;

    dilation(kernel_length);
}

void Filters::opening_o1(int kernel_length)
{
    erosion_o1(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif_;
    filtered_modif_ = previous_filtered_;
    previous_filtered_ = ptemp;

    dilation_o1(kernel_length);
}

void Filters::black_top_hat(int kernel_length)
{
    closing(kernel_length);

    for (int offset = 0; offset < byte_per_pixel_ * img_size_; offset++)
    {
        if (filtered_[offset] >= previous_filtered_[offset])
        {
            filtered_[offset] -= previous_filtered_[offset];
        }
        else
        {
            filtered_[offset] = 0;
        }
    }
}

void Filters::black_top_hat_o1(int kernel_length)
{
    closing_o1(kernel_length);

    for (int offset = 0; offset < byte_per_pixel_ * img_size_; offset++)
    {
        if (filtered_[offset] >= previous_filtered_[offset])
        {
            filtered_[offset] -= previous_filtered_[offset];
        }
        else
        {
            filtered_[offset] = 0;
        }
    }
}

void Filters::white_top_hat(int kernel_length)
{
    opening(kernel_length);

    for (int offset = 0; offset < byte_per_pixel_ * img_size_; offset++)
    {
        if (previous_filtered_[offset] >= filtered_[offset])
        {
            filtered_[offset] = previous_filtered_[offset] - filtered_[offset];
        }
        else
        {
            filtered_[offset] = 0;
        }
    }
}

void Filters::white_top_hat_o1(int kernel_length)
{
    opening_o1(kernel_length);

    for (int offset = 0; offset < byte_per_pixel_ * img_size_; offset++)
    {
        if (previous_filtered_[offset] >= filtered_[offset])
        {
            filtered_[offset] = previous_filtered_[offset] - filtered_[offset];
        }
        else
        {
            filtered_[offset] = 0;
        }
    }
}

void Filters::mean_filtering(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length - 1) / 2;
    int sum;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            sum = 0;

            // if the current pixel is not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        sum += int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                             color_channel]);
                    }
                }
            }
            // if in the border
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        if ((x + dx >= 0 && x + dx < img_width_) &&
                            (y + dy >= 0 && y + dy < img_height_))
                        {
                            sum +=
                                int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                              color_channel]);
                        }
                        if ((x + dx >= 0 && x + dx < img_width_) &&
                            (!(y + dy >= 0 && y + dy < img_height_)))
                        {
                            sum +=
                                int(filtered_[byte_per_pixel_ * ((x + dx) + (y - dy) * img_width_) +
                                              color_channel]);
                        }
                        if ((!(x + dx >= 0 && x + dx < img_width_)) &&
                            (y + dy >= 0 && y + dy < img_height_))
                        {
                            sum +=
                                int(filtered_[byte_per_pixel_ * ((x - dx) + (y + dy) * img_width_) +
                                              color_channel]);
                        }
                        if ((!(x + dx >= 0 && x + dx < img_width_)) &&
                            (!(y + dy >= 0 && y + dy < img_height_)))
                        {
                            sum +=
                                int(filtered_[byte_per_pixel_ * ((x - dx) + (y - dy) * img_width_) +
                                              color_channel]);
                        }
                    }
                }
            }
            filtered_modif_[byte_per_pixel_ * offset + color_channel] =
                (unsigned char)(sum / (kernel_length * kernel_length));
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

const float* Filters::gaussian_kernel(int kernel_length, float sigma)
{
    // to protect the input : kernel_length impair et strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    // to protect against /0
    if (sigma < 0.000000001f)
    {
        sigma = 0.000000001f;
    }

    const int kernel_size = kernel_length * kernel_length;

    float* kernel = new float[kernel_size];

    int x;
    int y;
    const int k = (kernel_length - 1) / 2;

    float sum = 0.f;

    for (int offset = 0; offset < kernel_size; offset++)
    {
        y = offset / kernel_length;
        x = offset - y * kernel_length;

        kernel[offset] = std::exp(-((float(y) - float(k)) * (float(y) - float(k)) +
                                    (float(x) - float(k)) * (float(x) - float(k))) /
                                  (2.f * sigma * sigma));

        sum += kernel[offset];
    }

    // to normalize
    for (int offset = 0; offset < kernel_size; offset++)
    {
        kernel[offset] = kernel[offset] / sum;
    }

    return kernel;
}

void Filters::gaussian_filtering(int kernel_length, float sigma)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    // to protect against /0
    if (sigma < 0.000000001f)
    {
        sigma = 0.000000001f;
    }

    const float* mask = gaussian_kernel(kernel_length, sigma);

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length - 1) / 2;
    float sum;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            sum = 0.f;

            // if the current pixel is not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        sum +=
                            mask[(kernel_radius + dx) + (kernel_radius + dy) * kernel_length] *
                            float(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                            color_channel]);
                    }
                }
            }
            // if is in the border
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        if ((x + dx >= 0 && x + dx < img_width_) &&
                            (y + dy >= 0 && y + dy < img_height_))
                        {
                            sum +=
                                mask[(kernel_radius + dx) + (kernel_radius + dy) * kernel_length] *
                                float(
                                    filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                              color_channel]);
                        }
                        if ((x + dx >= 0 && x + dx < img_width_) &&
                            (!(y + dy >= 0 && y + dy < img_height_)))
                        {
                            sum +=
                                mask[(kernel_radius + dx) + (kernel_radius + dy) * kernel_length] *
                                float(
                                    filtered_[byte_per_pixel_ * ((x + dx) + (y - dy) * img_width_) +
                                              color_channel]);
                        }
                        if ((!(x + dx >= 0 && x + dx < img_width_)) &&
                            (y + dy >= 0 && y + dy < img_height_))
                        {
                            sum +=
                                mask[(kernel_radius + dx) + (kernel_radius + dy) * kernel_length] *
                                float(
                                    filtered_[byte_per_pixel_ * ((x - dx) + (y + dy) * img_width_) +
                                              color_channel]);
                        }
                        if ((!(x + dx >= 0 && x + dx < img_width_)) &&
                            (!(y + dy >= 0 && y + dy < img_height_)))
                        {
                            sum +=
                                mask[(kernel_radius + dx) + (kernel_radius + dy) * kernel_length] *
                                float(
                                    filtered_[byte_per_pixel_ * ((x - dx) + (y - dy) * img_width_) +
                                              color_channel]);
                        }
                    }
                }
            }
            filtered_modif_[byte_per_pixel_ * offset + color_channel] = (unsigned char)(sum);
        }
    }

    delete[] mask;

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

void Filters::median_filtering_oNlogN(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length - 1) / 2;
    int m;

    const int length = kernel_length * kernel_length;
    unsigned char* const median_kernel = new unsigned char[length];

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            // m : index of the kernel
            m = 0;

            // if the current pixel is not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        median_kernel[m++] =
                            filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                      color_channel];
                    }
                }
            }
            // if in the border
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        if ((x + dx >= 0 && x + dx < img_width_) &&
                            (y + dy >= 0 && y + dy < img_height_))
                        {
                            median_kernel[m++] =
                                filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel];
                        }
                        if ((x + dx >= 0 && x + dx < img_width_) &&
                            (!(y + dy >= 0 && y + dy < img_height_)))
                        {
                            median_kernel[m++] =
                                filtered_[byte_per_pixel_ * ((x + dx) + (y - dy) * img_width_) +
                                          color_channel];
                        }
                        if ((!(x + dx >= 0 && x + dx < img_width_)) &&
                            (y + dy >= 0 && y + dy < img_height_))
                        {
                            median_kernel[m++] =
                                filtered_[byte_per_pixel_ * ((x - dx) + (y + dy) * img_width_) +
                                          color_channel];
                        }
                        if ((!(x + dx >= 0 && x + dx < img_width_)) &&
                            (!(y + dy >= 0 && y + dy < img_height_)))
                        {
                            median_kernel[m++] =
                                filtered_[byte_per_pixel_ * ((x - dx) + (y - dy) * img_width_) +
                                          color_channel];
                        }
                    }
                }
            }

            std::sort(median_kernel, median_kernel + length);

            filtered_modif_[byte_per_pixel_ * offset + color_channel] =
                median_kernel[(length - 1) / 2];
        }
    }
    delete[] median_kernel;

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

// Simon Perreault, Patrick Hébert: Median Filtering in Constant Time. IEEE Transactions on
// Image Processing 16(9): 2389-2394 (2007)
void Filters::median_filtering_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    const int median_rank = 1 + kernel_length * kernel_length / 2;

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length - 1) / 2;

    int I; // pixel intensity, grey-level or channel value for rgb image

    int x, y; // position of the current pixel

    int rank, m;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = 0; y < kernel_radius + 1; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius + 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while (rank < 1 + kernel_length * (x + 1) / 2)
                    {
                        rank += kernel_histo_[++I];
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] =
                        (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius + 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < median_rank)
                {
                    rank += kernel_histo_[++I];
                }

                filtered_modif_[find_offset(x, y, color_channel)] = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < 1 + kernel_length * (kernel_length - m) / 2)
                {
                    rank += kernel_histo_[++I];
                }

                filtered_modif_[find_offset(x, y, color_channel)] = (unsigned char)(I);
                m++;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for (I = 0; I < 256 * img_width_; I++)
        {
            columns_histo_[I] = 0;
        }

        // initialization
        for (x = 0; x < img_width_; x++)
        {
            for (y = 0; y < kernel_length; y++)
            {
                columns_histo_[256 * x + filtered_[find_offset(x, y, color_channel)]]++;
            }
        }

        // downward moving in the image
        for (y = kernel_radius + 1; y <= img_height_ - kernel_radius - 2; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y + kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while (rank < 1 + kernel_length * (x + 1) / 2)
                    {
                        rank += kernel_histo_[++I];
                    }

                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] =
                        (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y + kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < median_rank)
                {
                    rank += kernel_histo_[++I];
                }

                filtered_modif_[find_offset(x, y, color_channel)] = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < 1 + kernel_length * (kernel_length - m) / 2)
                {
                    rank += kernel_histo_[++I];
                }
                filtered_modif_[find_offset(x, y, color_channel)] = (unsigned char)(I);
                m++;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for (y = img_height_ - kernel_radius - 1; y < img_height_; y++)
        {
            // clear
            for (I = 0; I < 256; I++)
            {
                kernel_histo_[I] = 0;
            }

            // initialization for each new current row
            for (x = 0; x < kernel_length; x++)
            {
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius - 1, color_channel)]]--;
                columns_histo_[256 * x +
                               filtered_[find_offset(x, y - kernel_radius, color_channel)]]++;
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * x + I];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if (x >= kernel_radius)
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while (rank < 1 + kernel_length * (x + 1) / 2)
                    {
                        rank += kernel_histo_[++I];
                    }
                    filtered_modif_[find_offset(x - kernel_radius, y, color_channel)] =
                        (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for (x = kernel_radius + 1; x < img_width_ - kernel_radius - 1; x++)
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius - 1,
                                                     color_channel)]]--;
                columns_histo_[256 * (x + kernel_radius) +
                               filtered_[find_offset(x + kernel_radius, y - kernel_radius,
                                                     color_channel)]]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] += columns_histo_[256 * (x + kernel_radius) + I] -
                                        columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < median_rank)
                {
                    rank += kernel_histo_[++I];
                }

                filtered_modif_[find_offset(x, y, color_channel)] = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for (; x < img_width_; x++)
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius)
                // and H(x-kernel_radius-1)
                for (I = 0; I < 256; I++)
                {
                    kernel_histo_[I] -= columns_histo_[256 * (x - kernel_radius - 1) + I];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while (rank < 1 + kernel_length * (kernel_length - m) / 2)
                {
                    rank += kernel_histo_[++I];
                }

                filtered_modif_[find_offset(x, y, color_channel)] = (unsigned char)(I);
                m++;
            }
        }
    }

    if (kernel_length != 1)
    {
        // swap pointers
        unsigned char* ptemp = filtered_;
        filtered_ = filtered_modif_;
        filtered_modif_ = ptemp;
    }
}

void Filters::gaussian_white_noise(float sigma)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> nd{0.f, sigma};

    int filtered_int;

    for (int offset = 0; offset < byte_per_pixel_ * img_size_; offset++)
    {
        filtered_int = int(filtered_[offset]) + int(roundf(nd(gen)));

        if (filtered_int < 0)
        {
            filtered_int = 0;
        }
        else if (filtered_int > 255)
        {
            filtered_int = 255;
        }

        filtered_[offset] = (unsigned char)(filtered_int);
    }
}

void Filters::salt_pepper_noise(float proba)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution proba_distri{static_cast<double>(proba)};
    std::bernoulli_distribution coin_flip_distri{0.5};

    for (int offset = 0; offset < img_size_; offset++)
    {
        if (proba_distri(gen))
        {
            for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
            {
                if (coin_flip_distri(gen))
                {
                    filtered_[byte_per_pixel_ * offset + color_channel] = 0;
                }
                else
                {
                    filtered_[byte_per_pixel_ * offset + color_channel] = 255;
                }
            }
        }
    }
}

void Filters::speckle(float sigma)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> ud{0.f, 1.f};

    int filtered_int;

    for (int offset = 0; offset < byte_per_pixel_ * img_size_; offset++)
    {
        // img = img + X * img
        // with X a random variable from a uniform distribution centered at 0
        // and of a standard deviation sigma
        // sigma * sqrt(12) is the interval of the uniform distribution
        filtered_int =
            int(filtered_[offset]) +
            int(std::roundf(float(filtered_[offset]) * sigma * std::sqrt(12.f) * (ud(gen) - 0.5f)));

        if (filtered_int < 0)
        {
            filtered_int = 0;
        }
        else if (filtered_int > 255)
        {
            filtered_int = 255;
        }
        filtered_[offset] = (unsigned char)(filtered_int);
    }
}

void Filters::local_binary_pattern(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length - 1) / 2;

    int x, y; // position of the current pixel

    unsigned char Icentr;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            Icentr = filtered_[byte_per_pixel_ * offset + color_channel];

            filtered_modif_[byte_per_pixel_ * offset + color_channel] = 0;

            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                if (filtered_[byte_per_pixel_ *
                                  ((x - kernel_radius) + (y - kernel_radius) * img_width_) +
                              color_channel] >= Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 1;
                }
                if (filtered_[find_offset(x, y - kernel_radius, color_channel)] >= Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 2;
                }
                if (filtered_[find_offset(x + kernel_radius, y - kernel_radius, color_channel)] >=
                    Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 4;
                }
                if (filtered_[find_offset(x - kernel_radius, y, color_channel)] >= Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 8;
                }
                if (filtered_[byte_per_pixel_ * ((x + kernel_radius) + y * img_width_) +
                              color_channel] >= Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 16;
                }
                if (filtered_[byte_per_pixel_ *
                                  ((x - kernel_radius) + (y + kernel_radius) * img_width_) +
                              color_channel] >= Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 32;
                }
                if (filtered_[find_offset(x, y + kernel_radius, color_channel)] >= Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 64;
                }
                if (filtered_[find_offset(x + kernel_radius, y + kernel_radius, color_channel)] >=
                    Icentr)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] += 128;
                }
            }
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

void Filters::nagao_filtering(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if (kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if (kernel_length < 1)
    {
        kernel_length = 1;
    }

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length - 1) / 2;

    int var[9];
    int sum[9];

    int min_var;

    int nagao_size_init = math::square(kernel_radius + 1);
    if (nagao_size_init <= 0)
    {
        nagao_size_init = 1;
    }
    const int nagao_size = nagao_size_init;

    for (int color_channel = 0; color_channel < byte_per_pixel_; color_channel++)
    {
        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            for (int n = 0; n < 9; n++)
            {
                sum[n] = 0;
                var[n] = 0;
            }

            // if not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = 0; dy >= -kernel_radius; dy--)
                {
                    for (int dx = dy; dx <= -dy; dx++)
                    {
                        sum[0] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
                for (int dx = 0; dx <= kernel_radius; dx++)
                {
                    for (int dy = -dx; dy <= dx; dy++)
                    {
                        sum[1] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
                for (int dy = 0; dy <= kernel_radius; dy++)
                {
                    for (int dx = -dy; dx <= dy; dx++)
                    {
                        sum[2] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
                for (int dx = 0; dx >= -kernel_radius; dx--)
                {
                    for (int dy = dx; dy <= -dx; dy++)
                    {
                        sum[3] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }

                for (int dy = -kernel_radius; dy <= 0; dy++)
                {
                    for (int dx = 0; dx <= kernel_radius; dx++)
                    {
                        sum[4] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
                for (int dy = 0; dy <= kernel_radius; dy++)
                {
                    for (int dx = 0; dx <= kernel_radius; dx++)
                    {
                        sum[5] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
                for (int dy = 0; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= 0; dx++)
                    {
                        sum[6] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
                for (int dy = -kernel_radius; dy <= 0; dy++)
                {
                    for (int dx = -kernel_radius; dx <= 0; dx++)
                    {
                        sum[7] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }

                for (int dy = -kernel_radius + 1; dy <= kernel_radius - 1; dy++)
                {
                    for (int dx = -kernel_radius + 1; dx <= kernel_radius - 1; dx++)
                    {
                        sum[8] +=
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]);
                    }
                }
            }

            // if not in the border
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = 0; dy >= -kernel_radius; dy--)
                {
                    for (int dx = dy; dx <= -dy; dx++)
                    {
                        var[0] += math::square(
                            sum[0] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
                for (int dx = 0; dx <= kernel_radius; dx++)
                {
                    for (int dy = -dx; dy <= dx; dy++)
                    {
                        var[1] += math::square(
                            sum[1] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
                for (int dy = 0; dy <= kernel_radius; dy++)
                {
                    for (int dx = -dy; dx <= dy; dx++)
                    {
                        var[2] += math::square(
                            sum[2] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
                for (int dx = 0; dx >= -kernel_radius; dx--)
                {
                    for (int dy = dx; dy <= -dx; dy++)
                    {
                        var[3] += math::square(
                            sum[3] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }

                for (int dy = -kernel_radius; dy <= 0; dy++)
                {
                    for (int dx = 0; dx <= kernel_radius; dx++)
                    {
                        var[4] += math::square(
                            sum[4] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
                for (int dy = 0; dy <= kernel_radius; dy++)
                {
                    for (int dx = 0; dx <= kernel_radius; dx++)
                    {
                        var[5] += math::square(
                            sum[5] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
                for (int dy = 0; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= 0; dx++)
                    {
                        var[6] += math::square(
                            sum[6] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
                for (int dy = -kernel_radius; dy <= 0; dy++)
                {
                    for (int dx = -kernel_radius; dx <= 0; dx++)
                    {
                        var[7] += math::square(
                            sum[7] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }

                for (int dy = -kernel_radius + 1; dy <= kernel_radius - 1; dy++)
                {
                    for (int dx = -kernel_radius + 1; dx <= kernel_radius - 1; dx++)
                    {
                        var[8] += math::square(
                            sum[8] -
                            int(filtered_[byte_per_pixel_ * ((x + dx) + (y + dy) * img_width_) +
                                          color_channel]));
                    }
                }
            }

            min_var = 999999999;
            for (int n = 0; n < 9; n++)
            {
                if (var[n] < min_var)
                {
                    min_var = var[n];
                }
            }

            for (int n = 0; n < 9; n++)
            {
                if (var[n] == min_var)
                {
                    filtered_modif_[byte_per_pixel_ * offset + color_channel] =
                        (unsigned char)(sum[n] / nagao_size);
                }
            }
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered_;
    filtered_ = filtered_modif_;
    filtered_modif_ = ptemp;
}

//! Gradient morphologique avec un élement structurant carré de taille kernel_length
void Filters::morphological_gradient_yuv(int kernel_length, int alpha, int beta, int gamma)
{
    if (byte_per_pixel_ == 3)
    {
        // pour blinder l'entrée : kernel_length impair et strictement positif
        if (kernel_length % 2 == 0)
        {
            kernel_length--;
        }
        if (kernel_length < 1)
        {
            kernel_length = 1;
        }

        const int kernel_radius = (kernel_length - 1) / 2;

        int R, G, B;
        int Y, U, V;

        int x, y; // position of the current pixel

        int maxY, minY, maxU, minU, maxV, minV;

        for (int offset = 0; offset < img_size_; offset++)
        {
            y = offset / img_width_;
            x = offset - y * img_width_;

            // initialisation
            maxY = 0;
            minY = 255;
            maxU = 0;
            minU = 255;
            maxV = 0;
            minV = 255;

            // si pas sur le bord
            if (x > kernel_radius - 1 && x < img_width_ - kernel_radius && y > kernel_radius - 1 &&
                y < img_height_ - kernel_radius)
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        R = int(filtered_[4 * ((x + dx) + (y + dy) * img_width_) + 2]);
                        G = int(filtered_[4 * ((x + dx) + (y + dy) * img_width_) + 1]);
                        B = int(filtered_[static_cast<ptrdiff_t>(
                            4 * ((x + dx) + (y + dy) * img_width_))]);

                        Y = ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
                        U = ((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
                        V = ((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;

                        // pas de tests des voisins
                        if (Y > maxY)
                        {
                            maxY = Y;
                        }
                        if (Y < minY)
                        {
                            minY = Y;
                        }

                        if (U > maxU)
                        {
                            maxU = U;
                        }
                        if (U < minU)
                        {
                            minU = U;
                        }

                        if (V > maxV)
                        {
                            maxV = V;
                        }
                        if (V < minV)
                        {
                            minV = V;
                        }
                    }
                }
            }
            // si sur le bord
            else
            {
                for (int dy = -kernel_radius; dy <= kernel_radius; dy++)
                {
                    for (int dx = -kernel_radius; dx <= kernel_radius; dx++)
                    {
                        // tests des voisins
                        if (x + dx >= 0 && x + dx < img_width_ && y + dy >= 0 &&
                            y + dy < img_height_)
                        {
                            R = int(filtered_[4 * ((x + dx) + (y + dy) * img_width_) + 2]);
                            G = int(filtered_[4 * ((x + dx) + (y + dy) * img_width_) + 1]);
                            B = int(filtered_[static_cast<ptrdiff_t>(
                                4 * ((x + dx) + (y + dy) * img_width_))]);

                            Y = ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
                            U = ((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
                            V = ((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;

                            // pas de tests des voisins
                            if (Y > maxY)
                            {
                                maxY = Y;
                            }
                            if (Y < minY)
                            {
                                minY = Y;
                            }

                            if (U > maxU)
                            {
                                maxU = U;
                            }
                            if (U < minU)
                            {
                                minU = U;
                            }

                            if (V > maxV)
                            {
                                maxV = V;
                            }
                            if (V < minV)
                            {
                                minV = V;
                            }
                        }
                    }
                }
            }
            gradient_[offset] = (unsigned char)((alpha * (maxY - minY) + beta * (maxU - minU) +
                                                 gamma * (maxV - minV)) /
                                                (alpha + beta + gamma));
        }
    }
}

void Filters::fas(int kernel_length)
{
    for (int k = 3; k < kernel_length; k += 2)
    {
        opening(k);
        closing(k);
    }
}

} // namespace ofeli_ip
