// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once
#include <QCameraFormat>
#include <cmath>

namespace fluvel_app::camera_utils
{

inline bool isSameCameraFormat(const QCameraFormat& a, const QCameraFormat& b)
{
    constexpr double kFpsEpsilon = 0.01;

    return a.pixelFormat() == b.pixelFormat() && a.resolution() == b.resolution() &&
           std::abs(a.maxFrameRate() - b.maxFrameRate()) < kFpsEpsilon;
}

} // namespace fluvel_app::camera_utils
