# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

if(NOT FLUVEL_BUILD_BIND)
    return()
endif()

set(PYBIND11_FINDPYTHON ON)

find_package(
    Python
    REQUIRED

    COMPONENTS
        Interpreter
        Development
)

find_package(
    pybind11
    CONFIG
    REQUIRED
)

message(
    STATUS

    "Python executable : ${Python_EXECUTABLE}"
)

message(
    STATUS

    "Python version : ${Python_VERSION}"
)
