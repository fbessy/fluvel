// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file image_adapters.hpp
 * @brief Conversion utilities between fluvel_ip image types and QImage.
 *
 * This module provides a bridge between the core image processing layer
 * (ImageView, ImageOwner) and the Qt UI layer (QImage).
 *
 * Design principles:
 * - The core layer (fluvel_ip) is independent from Qt.
 * - ImageView is a non-owning, lightweight view used for algorithms.
 * - ImageOwner is an owning buffer used for storage and interop.
 * - QImage is used only at the UI / display boundary.
 *
 * Conversion strategies:
 * - toQImageCopy()   : safe, performs a deep copy (recommended default).
 * - toQImageUnsafe() : zero-copy, requires strict lifetime guarantees.
 * - toQImageShared() : zero-copy with shared ownership (safe for async usage).
 *
 * Usage guidelines:
 * - Use ImageView in algorithms and processing pipelines.
 * - Convert to QImage only at the UI or export boundary.
 * - Prefer copy-based conversion unless performance constraints require otherwise.
 */

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

#include <utility>

#ifdef FLUVEL_USE_QT
#include <QImage>
#include <memory>
#endif

#ifdef FLUVEL_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#ifdef FLUVEL_USE_QT

/**
 * @brief Create a non-owning ImageView from a QImage.
 *
 * This function provides a lightweight view over the pixel data stored in a QImage.
 * No data copy is performed. The returned ImageView directly references the
 * internal buffer of the QImage.
 *
 * @param img Source QImage.
 * @return ImageView referencing the QImage pixel data.
 *
 * @warning The returned ImageView is only valid as long as the QImage remains alive
 *          and its data is not modified or reallocated.
 *
 * @note This function is intended for read-only usage in image processing pipelines.
 *       Modifying the underlying QImage while the view is in use may lead to
 *       undefined behavior.
 */
fluvel_ip::ImageView imageViewFromQImage(const QImage& img);

/**
 * @brief Convert an ImageView to a QImage by performing a deep copy.
 *
 * This function allocates a new QImage and copies all pixel data from the
 * provided ImageView. The resulting QImage owns its memory and is completely
 * independent from the source.
 *
 * @param view Non-owning image view to convert.
 * @return QImage owning its internal buffer.
 *
 * @note This is the safest conversion method. No lifetime constraints.
 */
QImage toQImageCopy(const fluvel_ip::ImageView& view);

/**
 * @brief Convert an ImageOwner to a QImage without copying (unsafe).
 *
 * The returned QImage directly references the memory owned by the ImageOwner.
 * No data copy is performed.
 *
 * @param img ImageOwner providing the underlying buffer.
 * @return QImage referencing external memory.
 *
 * @warning The caller must ensure that the ImageOwner outlives the QImage.
 *          Using this function incorrectly may lead to undefined behavior.
 *
 * @note Use this function only in performance-critical paths where lifetime
 *       is strictly controlled.
 */
QImage toQImageUnsafe(const fluvel_ip::ImageOwner& img);

/**
 * @brief Convert an ImageOwner to a QImage using shared ownership (safe zero-copy).
 *
 * This function avoids copying pixel data while ensuring safe lifetime management.
 * The QImage references the underlying buffer and keeps it alive through a
 * shared_ptr.
 *
 * @param img Shared pointer to the ImageOwner.
 * @return QImage referencing shared memory.
 *
 * @note Suitable for multi-threaded or asynchronous pipelines.
 *       Provides zero-copy with safe lifetime management.
 */
QImage toQImageShared(std::shared_ptr<fluvel_ip::ImageOwner> img);

#endif // endif FLUVEL_USE_QT

#ifdef FLUVEL_USE_OPENCV
fluvel_ip::ImageView image_view_from_cvmat(const cv::Mat& mat);
#endif

#ifdef FLUVEL_USE_STB
fluvel_ip::ImageView image_view_from_stbi(const unsigned char* data, int width, int height,
                                          int channels);
#endif

#ifdef FLUVEL_USE_QT

inline fluvel_ip::ImageView imageViewFromQImage(const QImage& img)
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

inline QImage toQImageCopy(const fluvel_ip::ImageView& view)
{
    QImage img;

    switch (view.format())
    {
        case fluvel_ip::ImageFormat::Gray8:
        {
            img = QImage(view.width(), view.height(), QImage::Format_Grayscale8);

            for (int y = 0; y < view.height(); ++y)
            {
                std::memcpy(img.scanLine(y), view.row(y), view.width());
            }
            break;
        }

        case fluvel_ip::ImageFormat::Rgb24:
        {
            img = QImage(view.width(), view.height(), QImage::Format_RGB888);

            for (int y = 0; y < view.height(); ++y)
            {
                std::memcpy(img.scanLine(y), view.row(y), view.width() * 3);
            }
            break;
        }

        case fluvel_ip::ImageFormat::Bgr32:
        {
            img = QImage(view.width(), view.height(), QImage::Format_RGB32);

            for (int y = 0; y < view.height(); ++y)
            {
                std::memcpy(img.scanLine(y), view.row(y), view.width() * 4);
            }
            break;
        }

        default:
            return {};
    }

    return img;
}

inline QImage toQImageUnsafe(const fluvel_ip::ImageOwner& img)
{
    switch (img.format())
    {
        case fluvel_ip::ImageFormat::Rgb24:
            return QImage(img.data(), img.width(), img.height(), img.stride(),
                          QImage::Format_RGB888);

        case fluvel_ip::ImageFormat::Gray8:
            return QImage(img.data(), img.width(), img.height(), img.stride(),
                          QImage::Format_Grayscale8);

        case fluvel_ip::ImageFormat::Bgr32:
            return QImage(img.data(), img.width(), img.height(), img.stride(),
                          QImage::Format_RGB32);

        default:
            return {};
    }
}

inline QImage toQImageShared(std::shared_ptr<fluvel_ip::ImageOwner> img)
{
    QImage::Format qfmt = QImage::Format_Invalid;

    switch (img->format())
    {
        case fluvel_ip::ImageFormat::Gray8:
            qfmt = QImage::Format_Grayscale8;
            break;

        case fluvel_ip::ImageFormat::Rgb24:
            qfmt = QImage::Format_RGB888;
            break;

        case fluvel_ip::ImageFormat::Bgr32:
            qfmt = QImage::Format_RGB32;
            break;

        case fluvel_ip::ImageFormat::Rgba32:
            qfmt = QImage::Format_RGBA8888;
            break;

        default:
            return {};
    }

    // heap allocation pour garder le shared_ptr vivant
    auto* holder = new std::shared_ptr<fluvel_ip::ImageOwner>(std::move(img));

    return QImage((*holder)->data(), (*holder)->width(), (*holder)->height(), (*holder)->stride(),
                  qfmt,
                  [](void* p)
                  {
                      delete static_cast<std::shared_ptr<fluvel_ip::ImageOwner>*>(p);
                  },
                  holder);
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
