# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

cmake_minimum_required(VERSION 3.16)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(QT_CREATOR_SKIP_CONAN_SETUP ON)
endif()

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

set(FLUVEL_LOCAL_DEFAULT_FFMPEG OFF)

# =========================
# Video export option
# =========================

option(FLUVEL_ENABLE_CAMERA_SOURCE
       "Enable camera video source"
       ON)

option(FLUVEL_ENABLE_MEDIA_SOURCE
       "Enable media video source"
       ON)

option(FLUVEL_ENABLE_FFMPEG
       "Enable FFmpeg video export"
       ${FLUVEL_LOCAL_DEFAULT_FFMPEG})

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
include(cmake/fluvel_target_config.cmake)


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

    set(FLUVEL_USE_FFMPEG OFF)

    if(FLUVEL_ENABLE_FFMPEG)
        include(cmake/fluvel_ffmpeg.cmake)
    endif()

    include(cmake/fluvel_qt.cmake)
    set(CMAKE_AUTOMOC_VERBOSE ON)
    add_subdirectory(src/app)
endif()

message(STATUS
    "FFmpeg video export: requested=${FLUVEL_ENABLE_FFMPEG}, enabled=${FLUVEL_USE_FFMPEG}"
)

if(FLUVEL_ENABLE_FFMPEG AND NOT FLUVEL_USE_FFMPEG)

    if(FLUVEL_IS_CI)
        message(FATAL_ERROR
            "FFmpeg video export was requested but FFmpeg was not found."
        )
    else()
        message(WARNING
            "FFmpeg video export was requested but FFmpeg was not found. "
            "Building without FFmpeg support."
        )
    endif()

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