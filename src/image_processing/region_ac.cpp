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

namespace ofeli_ip
{

void RegionAc::do_specific_cycle1()
{
    if( pxl_nbr_out >= 1 )
    {
        average_out = (unsigned char)( sum_out /
                                       (unsigned int)pxl_nbr_out );
    }
    else
    {
        std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "Impossible to calculate average_out without outside points.";
    }

    if( pxl_nbr_total - pxl_nbr_out >= 1 )
    {
        average_in = (unsigned char)( (sum_total - sum_out) /
                                      (unsigned int)(pxl_nbr_total - pxl_nbr_out) );
    }
    else
    {
        std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "Impossible to calculate average_in without inside points.";
    }
}

void RegionAc::initialize_sums()
{
    sum_total = 0;

    sum_out = 0;
    pxl_nbr_out = 0;

    for( int offset = 0; offset < pxl_nbr_total; offset++ )
    {
        sum_total += image.pixel_at(offset);

        if( cd.get_phi()[offset] > PhiValue::ZERO_LEVEL_SET )
        {
            sum_out += image.pixel_at(offset);
            pxl_nbr_out++;
        }
    }
}

}
