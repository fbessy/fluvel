// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "py_array_conversion.hpp"

#include <stdexcept>

fluvel_ip::ImageView pyArrayToImageView(const py::array_t<uint8_t>& input)
{
    auto buf = input.request();

    const int h = static_cast<int>(buf.shape[0]);
    const int w = static_cast<int>(buf.shape[1]);
    const int c = static_cast<int>(buf.shape.size() == 3 ? buf.shape[2] : 1);
    const int stride = static_cast<int>(buf.strides[0]);

    const auto format = fluvel_ip::formatFromChannelCount(c);

    if (format == fluvel_ip::ImageFormat::Unknown)
    {
        throw std::invalid_argument("Unsupported NumPy image format. "
                                    "Expected uint8 grayscale or RGB image.");
    }

    return {static_cast<uint8_t*>(buf.ptr), w, h, format, stride};
}

py::array_t<uint8_t> imageOwnerToPyArray(const fluvel_ip::ImageOwner& image)
{
    return py::array_t<uint8_t>(
        {image.height(), image.width(), fluvel_ip::channelCount(image.format())}, image.data());
}
