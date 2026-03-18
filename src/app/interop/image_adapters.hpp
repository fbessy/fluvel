// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"

#include <utility>

#ifdef FLUVEL_USE_QT
#include <QImage>
#endif

#ifdef FLUVEL_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#ifdef FLUVEL_USE_QT
fluvel_ip::ImageView image_view_from_qimage(const QImage& img);
#endif

#ifdef FLUVEL_USE_OPENCV
fluvel_ip::ImageView image_view_from_cvmat(const cv::Mat& mat);
#endif

#ifdef FLUVEL_USE_STB
fluvel_ip::ImageView image_view_from_stbi(const unsigned char* data, int width, int height,
                                          int channels);
#endif

#ifdef FLUVEL_USE_QT
inline fluvel_ip::ImageView image_view_from_qimage(const QImage& img)
{
    assert(!img.isNull());

    const int w = img.width();
    const int h = img.height();
    const int stride = static_cast<int>(img.bytesPerLine());
    const auto* data = reinterpret_cast<const unsigned char*>(img.constBits());

    switch (img.format())
    {
        case QImage::Format_Grayscale8:
            return fluvel_ip::ImageView(data, w, h, fluvel_ip::ImageFormat::Gray8, stride);

        case QImage::Format_RGB888:
            return fluvel_ip::ImageView(data, w, h, fluvel_ip::ImageFormat::Rgb24, stride);

        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
            // Qt stocke BGRA en mémoire
            return fluvel_ip::ImageView(data, w, h, fluvel_ip::ImageFormat::Bgr32, stride);

        default:
            assert(false && "Unsupported QImage format");
            std::unreachable();
    }
}
#endif

#ifdef FLUVEL_USE_OPENCV
inline fluvel_ip::ImageView image_view_from_cvmat(const cv::Mat& mat)
{
    assert(!mat.empty());
    assert(mat.depth() == CV_8U);

    const int w = mat.cols;
    const int h = mat.rows;
    const int stride = static_cast<int>(mat.step);
    const auto* data = reinterpret_cast<const unsigned char*>(mat.data);

    switch (mat.type())
    {
        case CV_8UC1:
            return fluvel_ip::ImageView(data, w, h, fluvel_ip::ImageFormat::Gray8, stride);

        case CV_8UC3:
            // OpenCV = BGR en mémoire
            return fluvel_ip::ImageView(data, w, h, fluvel_ip::ImageFormat::Bgr24, stride);

        case CV_8UC4:
            return fluvel_ip::ImageView(data, w, h, fluvel_ip::ImageFormat::Bgr32, stride);

        default:
            assert(false && "Unsupported cv::Mat type");
            std::unreachable();
    }
}
#endif

#ifdef FLUVEL_USE_STB
inline fluvel_ip::ImageView image_view_from_stbi(const unsigned char* data, int width, int height,
                                                 int channels)
{
    assert(data != nullptr);
    assert(width > 0);
    assert(height > 0);
    assert(channels == 1 || channels == 3 || channels == 4);

    switch (channels)
    {
        case 1:
            return fluvel_ip::ImageView(data, width, height, fluvel_ip::ImageFormat::Gray8,
                                        width); // compact

        case 3:
            // stb = RGB
            return fluvel_ip::ImageView(data, width, height, fluvel_ip::ImageFormat::Rgb24,
                                        width * 3);

        case 4:
            // stb = RGBA
            return fluvel_ip::ImageView(data, width, height, fluvel_ip::ImageFormat::Rgba32,
                                        width * 4);

        default:
            assert(false && "Unsupported stb_image channel count");
            std::unreachable();
    }
}
#endif
