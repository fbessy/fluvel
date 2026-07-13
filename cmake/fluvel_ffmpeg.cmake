# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

if(NOT FLUVEL_BUILD_APP)
    return()
endif()

find_package(PkgConfig QUIET)

set(FLUVEL_USE_FFMPEG OFF)

if(PkgConfig_FOUND)

    pkg_check_modules(AVFORMAT QUIET libavformat)
    pkg_check_modules(AVCODEC QUIET libavcodec)
    pkg_check_modules(AVUTIL QUIET libavutil)
    pkg_check_modules(SWSCALE QUIET libswscale)

    if(AVFORMAT_FOUND
       AND AVCODEC_FOUND
       AND AVUTIL_FOUND
       AND SWSCALE_FOUND)

        set(FLUVEL_USE_FFMPEG ON)

    endif()

endif()
