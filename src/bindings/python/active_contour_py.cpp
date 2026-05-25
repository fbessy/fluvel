// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "active_contour_py.hpp"

#include <pybind11/numpy.h>

#include "active_contour.hpp"
#include "contour_data.hpp"
#include "contour_types.hpp"
#include "region_color_speed_model.hpp"
#include "region_gray_speed_model.hpp"
#include "speed_model.hpp"

#include "contour_conversion.hpp"
#include "py_array_conversion.hpp"

namespace
{

struct ActiveContourResult
{
    py::array outer;
    py::array inner;
};

} // namespace

void bindActiveContour(py::module_& m)
{
    py::class_<ActiveContourResult>(m, "ActiveContourResult")

        .def_readonly("outer", &ActiveContourResult::outer)

        .def_readonly("inner", &ActiveContourResult::inner);

    m.def(
        "active_contour",

        [](py::array_t<uint8_t> image)
        {
            auto img = pyArrayToImageView(image);

            fluvel_ip::ContourData cd(img.width(), img.height());

            std::unique_ptr<fluvel_ip::ISpeedModel> model;

            if (img.format() == fluvel_ip::ImageFormat::Gray8)
            {
                model = std::make_unique<fluvel_ip::RegionGraySpeedModel>();
            }
            else
            {
                model = std::make_unique<fluvel_ip::RegionColorSpeedModel>();
            }

            fluvel_ip::ActiveContour ac(std::move(cd), std::move(model));

            ac.update(img);

            ac.converge();

            return ActiveContourResult{contourToPyArray(ac.outerBoundary()),

                                       contourToPyArray(ac.innerBoundary())};
        },

        py::arg("image"));
}