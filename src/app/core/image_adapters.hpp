#ifndef IMAGE_ADAPTERS_HPP
#define IMAGE_ADAPTERS_HPP

#include "image_span.hpp"

#ifdef OFELI_HAS_QT
#include <QImage>
#endif

#ifdef OFELI_HAS_OPEN_CV
#include <opencv2/core.hpp>
#include "core/image_span.hpp"
#endif

namespace ofeli_app
{

#ifdef OFELI_HAS_QT

inline ofeli_ip::ImageSpan8 image_span_8_from_qimage(const QImage& img)
{
    assert(img.format() == QImage::Format_Grayscale8);

    return ofeli_ip::ImageSpan8( img.constBits(),
                                      img.width(),
                                      img.height() );
}

inline ofeli_ip::ImageSpan32 image_span_32_from_qimage(const QImage& img)
{
    assert(img.format() == QImage::Format_ARGB32 ||
           img.format() == QImage::Format_RGB32);

    return ofeli_ip::ImageSpan32( img.constBits(),
                                       img.width(),
                                       img.height() );
}

#endif

#ifdef OFELI_HAS_OPEN_CV

inline ofeli_ip::ImageSpan8 image8_view_from_cvmat(const cv::Mat& mat)
{
    assert(mat.type() == CV_8UC1);

    return ofeli_ip::ImageSpan8( mat.data,
                                      mat.cols,
                                      mat.rows );
}

inline ofeli_ip::ImageSpan32 image32_view_from_cvmat(const cv::Mat& mat)
{
    assert(mat.type() == CV_8UC4);

    return ofeli_ip::ImageSpan32( mat.data,
                                       mat.cols,
                                       mat.rows );
}


#endif

}

#endif // IMAGE_ADAPTERS_HPP
