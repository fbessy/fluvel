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

#include "region_ac.hpp"
#include "ofeli_math.hpp"

namespace ofeli_ip
{

void RegionAc::initialize_sums()
{
    sum_total_ = 0;

    sum_out_ = 0;
    pxl_nbr_out_ = 0;

    for( int offset = 0; offset < pxl_nbr_total_; ++offset )
    {
        const int64_t intensity = static_cast<int64_t>( image_.pixel_at(offset) );

        sum_total_ += intensity;

        if( phi_value::isOutside( cd_.phi()[offset] ) )
        {
            sum_out_ += intensity;
            ++pxl_nbr_out_;
        }
    }
}

void RegionAc::do_specific_cycle1()
{
    if( pxl_nbr_out_ >= 1 )
        average_out_ = lround( static_cast<float>(sum_out_) / pxl_nbr_out_ );


    const int64_t sum_in = sum_total_ - sum_out_;
    const int64_t pxl_nbr_in = pxl_nbr_total_ - pxl_nbr_out_;

    if( pxl_nbr_in >= 1 )
        average_in_ = lround( static_cast<float>(sum_in) / pxl_nbr_in );
}

void RegionAc::compute_external_speed_Fd(ContourPoint& point)
{
    const int pxl = static_cast<int>( image_.pixel_at( point.offset() ) );

    const int diff_out = pxl - average_out_;
    const int diff_in  = pxl - average_in_;

    const int speed =   region_config_.lambda_out * ( math::square(diff_out) )
                      - region_config_.lambda_in  * ( math::square(diff_in) );

    point.set_speed( speed_value::get_discrete_speed( speed ) );
}

void RegionAc::do_specific_when_switch(int offset,
                                       BoundarySwitch ctx_choice)
{
    const int64_t intensity = static_cast<int64_t>( image_.pixel_at(offset) );

    if ( ctx_choice == BoundarySwitch::In )
    {
        sum_out_ -= intensity;
        --pxl_nbr_out_;
    }
    else if ( ctx_choice == BoundarySwitch::Out )
    {
        sum_out_ += intensity;
        ++pxl_nbr_out_;
    }
}

}
