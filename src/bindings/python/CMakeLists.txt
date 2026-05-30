# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

# current binding python directory
set(PY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(INTEROP_DIR         ${PY_DIR}/interop)

set(python_src

    module.cpp

    noise_py.cpp
    noise_py.hpp

    filtering_py.cpp
    filtering_py.hpp

    morpho_py.cpp
    morpho_py.hpp

    active_contour_py.cpp
    active_contour_py.hpp

    py_validation.cpp
    py_validation.hpp

    ${INTEROP_DIR}/py_array_conversion.cpp
    ${INTEROP_DIR}/py_array_conversion.hpp

    ${INTEROP_DIR}/contour_conversion.cpp
    ${INTEROP_DIR}/contour_conversion.hpp
)

pybind11_add_module(
    ${PYTHON_TARGET_NAME}
    ${python_src}
)

target_link_libraries(
    ${PYTHON_TARGET_NAME}

    PRIVATE
        fluvel::image_processing
)

target_include_directories(${PYTHON_TARGET_NAME} PRIVATE
    ${INTEROP_DIR}
    ${PROJECT_SOURCE_DIR}/src/image_processing
)

set_target_properties(
    ${PYTHON_TARGET_NAME}

    PROPERTIES

        LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/python

        OUTPUT_NAME
            "fluvel"
)