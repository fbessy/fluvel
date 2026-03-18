// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_adapters.hpp"

std::optional<VideoFrame> video_frame_from_qvideoframe(QVideoFrame frame)
{
    if (!frame.isValid())
        return std::nullopt;

    if (!frame.map(QVideoFrame::ReadOnly))
        return std::nullopt;

    VideoFrame out;
    out.frame = frame;

    const int W = frame.width();
    const int H = frame.height();

    out.width = W;
    out.height = H;

    const auto fmt = frame.pixelFormat();

    // ---------------- YUYV (packed)
    if (fmt == QVideoFrameFormat::Format_YUYV)
    {
        const int stride = frame.bytesPerLine(0);
        const int size = stride * H;

        out.buffer.resize(size);
        std::memcpy(out.buffer.data(), frame.bits(0), size);

        out.format = fluvel_ip::ImageFormat::Yuyv422;

        out.span = fluvel_ip::ImageSpan(out.buffer.data(), W, H, out.format, stride);
    }

    // ---------------- NV12 / NV21 (semi-planar)
    else if (fmt == QVideoFrameFormat::Format_NV12 || fmt == QVideoFrameFormat::Format_NV21)
    {
        const int Y_size = W * H;
        const int UV_size = W * H / 2;

        out.buffer.resize(Y_size + UV_size);

        // plane 0 = Y
        std::memcpy(out.buffer.data(), frame.bits(0), Y_size);

        // plane 1 = UV
        std::memcpy(out.buffer.data() + Y_size, frame.bits(1), UV_size);

        out.format = (fmt == QVideoFrameFormat::Format_NV12) ? fluvel_ip::ImageFormat::Nv12
                                                             : fluvel_ip::ImageFormat::Nv21;

        out.span = fluvel_ip::ImageSpan(out.buffer.data(), W, H, out.format, 0);
    }

    // ---------------- I420 / YUV420P / YV12 (planar)
    else if (fmt == QVideoFrameFormat::Format_YUV420P || fmt == QVideoFrameFormat::Format_YV12)
    {
        const int Y_size = W * H;
        const int UV_size = (W / 2) * (H / 2);

        out.buffer.resize(Y_size + 2 * UV_size);

        unsigned char* dst = out.buffer.data();

        const unsigned char* Y_src = frame.bits(0);
        const unsigned char* U_src = frame.bits(1);
        const unsigned char* V_src = frame.bits(2);

        // Y
        std::memcpy(dst, Y_src, Y_size);

        // ⚠️ attention YV12 = V puis U
        if (fmt == QVideoFrameFormat::Format_YUV420P)
        {
            std::memcpy(dst + Y_size, U_src, UV_size);
            std::memcpy(dst + Y_size + UV_size, V_src, UV_size);
        }
        else // YV12
        {
            std::memcpy(dst + Y_size, V_src, UV_size);
            std::memcpy(dst + Y_size + UV_size, U_src, UV_size);
        }

        out.format = fluvel_ip::ImageFormat::I420;

        out.span = fluvel_ip::ImageSpan(out.buffer.data(), W, H, out.format, 0);
    }

    else
    {
        frame.unmap();
        return std::nullopt;
    }

    frame.unmap();
    return out;
}
