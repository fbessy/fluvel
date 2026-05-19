# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE

    $<$<BOOL:${FLUVEL_PLATFORM_DESKTOP}>:
        FLUVEL_PLATFORM_DESKTOP>

    $<$<BOOL:${FLUVEL_PLATFORM_MOBILE}>:
        FLUVEL_PLATFORM_MOBILE>

    $<$<BOOL:${FLUVEL_PLATFORM_WINDOWS}>:
        FLUVEL_PLATFORM_WINDOWS>

    $<$<BOOL:${FLUVEL_PLATFORM_LINUX}>:
        FLUVEL_PLATFORM_LINUX>

    $<$<BOOL:${FLUVEL_PLATFORM_MACOS}>:
        FLUVEL_PLATFORM_MACOS>

    $<$<BOOL:${FLUVEL_PLATFORM_ANDROID}>:
        FLUVEL_PLATFORM_ANDROID>

    $<$<BOOL:${FLUVEL_PLATFORM_IOS}>:
        FLUVEL_PLATFORM_IOS>
)

if(OpenCV_FOUND)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_USE_OPENCV)
endif()

if(Qt6_FOUND)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_USE_QT)
endif()

if(FLUVEL_UI_DESKTOP)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_UI_DESKTOP)
endif()

if(ENABLE_ASAN)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_ASAN)
else()
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:FLUVEL_DEBUG>
    )
endif()
