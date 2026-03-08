// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "edge_ac.hpp"
#include "fluvel_math.hpp"

#include <cstring>

namespace fluvel_ip
{

constexpr auto kGrayscaleDepth = 256u;

void EdgeAc::computeExternalSpeedFd(ContourPoint& point)
{
    const int x = point.x();
    const int y = point.y();

    int max_out = 0;
    int max_in = 0;

    int dd;

    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            if (cd_.phi().valid(x + dx, y + dy))
            {
                dd = std::abs(int(gradient_image_.at(x, y)) -
                              int(gradient_image_.at(x + dx, y + dy)));

                if (phi_value::isOutside(cd_.phi().at(x + dx, y + dy)))
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

    int global_speed = global_speed_sign_ * (int(threshold_) - int(gradient_image_.at(x, y)));

    point.speed_ = speed_value::get_discrete_speed(3 * local_speed + global_speed);
}

int EdgeAc::get_global_speed_sign() const
{
    int sign = 0;
    unsigned int sum_in = 0;
    unsigned int sum_out = 0;

    const int w = gradient_image_.width();
    const int h = gradient_image_.height();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            if (phi_value::isInside(cd_.phi().at(x, y)))
            {
                sum_in += (unsigned int)(gradient_image_.at(x, y));
            }
            else
            {
                sum_out += (unsigned int)(gradient_image_.at(x, y));
            }
        }
    }

    if (sum_out > sum_in)
    {
        sign = 1;
    }
    else
    {
        sign = -1;
    }

    return sign;
}

unsigned char EdgeAc::do_otsu_method(ImageSpan image)
{
    unsigned char threshold = 0;

    unsigned int histogram[kGrayscaleDepth];

    std::memset((void*)histogram, 0, kGrayscaleDepth * sizeof(unsigned int));

    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            histogram[image.gray(x, y)]++;
        }
    }

    unsigned int sum = 0;
    for (unsigned int intensity = 0; intensity < kGrayscaleDepth; intensity++)
    {
        sum += intensity * histogram[intensity];
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

    while (t < (kGrayscaleDepth - 1) && weight2 != 0)
    {
        weight1 += histogram[t];
        weight2 = image.size() - weight1;

        if (weight1 != 0 && weight2 != 0)
        {
            sum1 += t * histogram[t];

            mean1 = (unsigned char)(sum1 / weight1);
            mean2 = (unsigned char)((sum - sum1) / weight2); // sum2 = sum-sum1

            var_t = (float)weight1 * (float)weight2 * (float)math::square(int(mean1) - int(mean2));

            if (var_t > var_max)
            {
                var_max = var_t;
                threshold = t;
            }
        }

        t++;
    }

    return threshold;
}

} // namespace fluvel_ip
