#ifndef IMAGE_ADAPTERS_HPP
#define IMAGE_ADAPTERS_HPP

#include "image_views.hpp"

#ifdef OFELI_HAS_QT
#include <QImage>
#endif

#ifdef OFELI_HAS_OPEN_CV
#include <opencv2/core.hpp>
#include "core/image_views.hpp"
#endif

namespace ofeli_gui
{

#ifdef OFELI_HAS_QT

inline ofeli_ip::Image8ConstView image8_view_from_qimage(const QImage& img)
{
    //assert(img.format() == QImage::Format_Grayscale8);

    return ofeli_ip::Image8ConstView( img.constBits(),
                                      img.width(),
                                      img.height() );
}

inline ofeli_ip::Image32ConstView image32_view_from_qimage(const QImage& img)
{
    //assert(img.format() == QImage::Format_ARGB32 ||
           //img.format() == QImage::Format_RGB32);

    return ofeli_ip::Image32ConstView( img.constBits(),
                                       img.width(),
                                       img.height() );
}

#endif

#ifdef OFELI_HAS_OPEN_CV

inline ofeli_ip::Image8ConstView image8_view_from_cvmat(const cv::Mat& mat)
{
    assert(mat.type() == CV_8UC1);

    return ofeli_ip::Image8ConstView( mat.data,
                                      mat.cols,
                                      mat.rows );
}

inline ofeli_ip::Image32ConstView image32_view_from_cvmat(const cv::Mat& mat)
{
    assert(mat.type() == CV_8UC4);

    return ofeli_ip::Image32ConstView( mat.data,
                                       mat.cols,
                                       mat.rows );
}


#endif

}

#endif // IMAGE_ADAPTERS_HPP
