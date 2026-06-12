#include "video_format_utils.hpp"

#include <QObject>

namespace fluvel::video_utils
{

QString pixelFormatToString(QVideoFrameFormat::PixelFormat format)
{
    using PF = QVideoFrameFormat::PixelFormat;

    switch (format)
    {
        case PF::Format_Invalid:
            return "Invalid";

        case PF::Format_ARGB8888:
            return "ARGB8888";

        case PF::Format_ARGB8888_Premultiplied:
            return "ARGB8888_Premultiplied";

        case PF::Format_XRGB8888:
            return "XRGB8888";

        case PF::Format_BGRA8888:
            return "BGRA8888";

        case PF::Format_BGRA8888_Premultiplied:
            return "BGRA8888_Premultiplied";

        case PF::Format_BGRX8888:
            return "BGRX8888";

        case PF::Format_ABGR8888:
            return "ABGR8888";

        case PF::Format_XBGR8888:
            return "XBGR8888";

        case PF::Format_RGBA8888:
            return "RGBA8888";

        case PF::Format_RGBX8888:
            return "RGBX8888";

        case PF::Format_AYUV:
            return "AYUV";

        case PF::Format_AYUV_Premultiplied:
            return "AYUV_Premultiplied";

        case PF::Format_YUV420P:
            return "YUV420P";

        case PF::Format_YUV422P:
            return "YUV422P";

        case PF::Format_YV12:
            return "YV12";

        case PF::Format_UYVY:
            return "UYVY";

        case PF::Format_YUYV:
            return "YUYV";

        case PF::Format_NV12:
            return "NV12";

        case PF::Format_NV21:
            return "NV21";

        case PF::Format_IMC1:
            return "IMC1";

        case PF::Format_IMC2:
            return "IMC2";

        case PF::Format_IMC3:
            return "IMC3";

        case PF::Format_IMC4:
            return "IMC4";

        case PF::Format_Y8:
            return "Y8";

        case PF::Format_Y16:
            return "Y16";

        case PF::Format_P010:
            return "P010";

        case PF::Format_P016:
            return "P016";

        case PF::Format_SamplerExternalOES:
            return "SamplerExternalOES";

        case PF::Format_Jpeg:
            return "JPEG";

        case PF::Format_SamplerRect:
            return "SamplerRect";

        case PF::Format_YUV420P10:
            return "YUV420P10";
    }

    return QString(QObject::tr("Unknown(%1)")).arg(static_cast<int>(format));
}

QString pixelFormatToShortString(QVideoFrameFormat::PixelFormat format)
{
    switch (format)
    {
        case QVideoFrameFormat::Format_NV12:
            return "NV12";
        case QVideoFrameFormat::Format_NV21:
            return "NV21";
        case QVideoFrameFormat::Format_YUV420P:
            return "YUV420";
        case QVideoFrameFormat::Format_YUYV:
            return "YUYV";
        case QVideoFrameFormat::Format_Jpeg:
            return "MJPEG";
        default:
            return QObject::tr("Other");
    }
}

} // namespace fluvel::video_utils