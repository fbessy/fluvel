// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_span.hpp"

#include <utility>

#ifdef FLUVEL_USE_QT
#include <QImage>
#endif

#ifdef FLUVEL_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#ifdef FLUVEL_USE_QT
ofeli_ip::ImageSpan image_span_from_qimage(const QImage& img);
#endif

#ifdef FLUVEL_USE_OPENCV
ofeli_ip::ImageSpan image_span_from_cvmat(const cv::Mat& mat);
#endif

#ifdef OFELI_USE_STB
ofeli_ip::ImageSpan image_span_from_stbi(const unsigned char* data, int width, int height,
                                         int channels);
#endif

#ifdef FLUVEL_USE_QT
inline ofeli_ip::ImageSpan image_span_from_qimage(const QImage& img)
{
    assert(!img.isNull());

    const int w = img.width();
    const int h = img.height();
    const int stride = static_cast<int>(img.bytesPerLine());
    const auto* data = reinterpret_cast<const unsigned char*>(img.constBits());

    switch (img.format())
    {
        case QImage::Format_Grayscale8:
            return ofeli_ip::ImageSpan(data, w, h, ofeli_ip::ImageFormat::Gray8, stride);

        case QImage::Format_RGB888:
            return ofeli_ip::ImageSpan(data, w, h, ofeli_ip::ImageFormat::Rgb24, stride);

        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
            // Qt stocke BGRA en mémoire
            return ofeli_ip::ImageSpan(data, w, h, ofeli_ip::ImageFormat::Bgr32, stride);

        default:
            assert(false && "Unsupported QImage format");
            std::unreachable();
    }
}
#endif

#ifdef FLUVEL_USE_OPENCV
inline ofeli_ip::ImageSpan image_span_from_cvmat(const cv::Mat& mat)
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
            return ofeli_ip::ImageSpan(data, w, h, ofeli_ip::ImageFormat::Gray8, stride);

        case CV_8UC3:
            // OpenCV = BGR en mémoire
            return ofeli_ip::ImageSpan(data, w, h, ofeli_ip::ImageFormat::Bgr24, stride);

        case CV_8UC4:
            return ofeli_ip::ImageSpan(data, w, h, ofeli_ip::ImageFormat::Bgr32, stride);

        default:
            assert(false && "Unsupported cv::Mat type");
            std::unreachable();
    }
}
#endif

#ifdef OFELI_USE_STB
inline ofeli_ip::ImageSpan image_span_from_stbi(const unsigned char* data, int width, int height,
                                                int channels)
{
    assert(data != nullptr);
    assert(width > 0);
    assert(height > 0);
    assert(channels == 1 || channels == 3 || channels == 4);

    switch (channels)
    {
        case 1:
            return ofeli_ip::ImageSpan(data, width, height, ofeli_ip::ImageFormat::Gray8,
                                       width); // compact

        case 3:
            // stb = RGB
            return ofeli_ip::ImageSpan(data, width, height, ofeli_ip::ImageFormat::Rgb24,
                                       width * 3);

        case 4:
            // stb = RGBA
            return ofeli_ip::ImageSpan(data, width, height, ofeli_ip::ImageFormat::Rgba32,
                                       width * 4);

        default:
            assert(false && "Unsupported stb_image channel count");
            std::unreachable();
    }
}
#endif
