# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

if(NOT FLUVEL_BUILD_APP)
    return()
endif()

set(FLUVEL_USE_FFMPEG OFF)

find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)

    pkg_check_modules(FFMPEG QUIET IMPORTED_TARGET
        libavformat
        libavcodec
        libavutil
        libswscale
    )

    if(FFMPEG_FOUND)
        set(FLUVEL_USE_FFMPEG ON)
    endif()

endif()
