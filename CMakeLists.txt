# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

cmake_minimum_required(VERSION 3.16)

project(Fluvel VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

# =========================
# Default build products
# Modify if needed
# =========================

set(FLUVEL_DEFAULT_BUILD_APP ON)
set(FLUVEL_DEFAULT_BUILD_BIND OFF)

# =========================
# Local development defaults
# Modify here if needed
# =========================

set(FLUVEL_LOCAL_DEFAULT_CCACHE OFF)
set(FLUVEL_LOCAL_DEFAULT_UNITY OFF)

set(FLUVEL_LOCAL_DEFAULT_ASAN OFF)
set(FLUVEL_LOCAL_DEFAULT_TSAN OFF)

# =========================
# Experimental UI options
# =========================

option(FLUVEL_USE_QML
       "Enable experimental QML UI"
       OFF)

option(FLUVEL_UI_DESKTOP
       "Use QWidget UI on Mobile"
       ON)

include(cmake/fluvel_platforms.cmake)
include(cmake/fluvel_build_config.cmake)

set(FLUVEL_ROOT_DIR
    ${PROJECT_SOURCE_DIR}
)

set(FLUVEL_RESOURCES_DIR
    ${FLUVEL_ROOT_DIR}/resources
)

set(FLUVEL_TRANSLATIONS_DIR
    ${FLUVEL_ROOT_DIR}/translations
)

set(FLUVEL_PACKAGING_DIR
    ${FLUVEL_ROOT_DIR}/packaging
)

set(LIB_TARGET_NAME fluvel_ip)
set(APP_TARGET_NAME fluvel_app)
set(PYTHON_TARGET_NAME fluvel_py)

# =========================
# Modules
# =========================

add_subdirectory(src/image_processing)

if(FLUVEL_BUILD_APP)
    include(cmake/fluvel_qt.cmake)
    set(CMAKE_AUTOMOC_VERBOSE ON)
    add_subdirectory(src/app)
endif()

if(FLUVEL_BUILD_BIND)
    include(cmake/fluvel_python.cmake)
    add_subdirectory(src/bindings)
endif()

# =========================
# Packaging
# =========================

if(FLUVEL_BUILD_APP)
    include(cmake/fluvel_package.cmake)
endif()