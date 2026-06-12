// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_format_utils.hpp"
#include "video_format_utils.hpp"

namespace fluvel::camera_utils
{

bool isSameCameraFormat(const QCameraFormat& a, const QCameraFormat& b)
{
    constexpr double kFpsEpsilon = 0.01;

    return a.pixelFormat() == b.pixelFormat() && a.resolution() == b.resolution() &&
           std::abs(a.maxFrameRate() - b.maxFrameRate()) < kFpsEpsilon;
}

bool isYuv420(QVideoFrameFormat::PixelFormat format)
{
    return format == QVideoFrameFormat::Format_YUV420P ||
           format == QVideoFrameFormat::Format_NV12 || format == QVideoFrameFormat::Format_NV21;
}

bool isYuv(QVideoFrameFormat::PixelFormat format)
{
    return isYuv420(format) || format == QVideoFrameFormat::Format_YUYV;
}

bool is640x480(const QSize& size)
{
    return size.width() == 640 && size.height() == 480;
}

bool is30fps(float fps)
{
    return std::abs(fps - 30.0f) < 1.0f;
}

int findBestFormatIndex(const QList<QCameraFormat>& formats)
{
    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv420(f.pixelFormat()) && is640x480(f.resolution()) && is30fps(f.maxFrameRate()))
            return i;
    }

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv(f.pixelFormat()) && is640x480(f.resolution()) && is30fps(f.maxFrameRate()))
            return i;
    }

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv(f.pixelFormat()) && is640x480(f.resolution()))
            return i;
    }

    for (int i = 0; i < formats.size(); ++i)
    {
        const auto& f = formats[i];
        if (isYuv(f.pixelFormat()))
            return i;
    }

    return -1;
}

QString formatToString(const QCameraFormat& format)
{
    const QSize res = format.resolution();
    const int fps = static_cast<int>(format.maxFrameRate());

    const QString pixelFormat = video_utils::pixelFormatToShortString(format.pixelFormat());

    return QString("%1 %2x%3 @%4").arg(pixelFormat).arg(res.width()).arg(res.height()).arg(fps);
}

} // namespace fluvel::camera_utils