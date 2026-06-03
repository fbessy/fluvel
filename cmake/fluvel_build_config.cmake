# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy


# =========================
# Build products
# =========================

option(
    FLUVEL_BUILD_APP
    "Build desktop/mobile application"
    ${FLUVEL_DEFAULT_BUILD_APP}
)

option(
    FLUVEL_BUILD_BIND
    "Build language bindings"
    ${FLUVEL_DEFAULT_BUILD_BIND}
)

message(STATUS "Build app      : ${FLUVEL_BUILD_APP}")
message(STATUS "Build bindings : ${FLUVEL_BUILD_BIND}")

set(FLUVEL_IS_CI FALSE)

if(DEFINED ENV{CI})
    set(FLUVEL_IS_CI TRUE)
endif()

if(FLUVEL_IS_CI)
    set(FLUVEL_DEFAULT_CCACHE OFF)
    set(FLUVEL_DEFAULT_UNITY OFF)
    set(FLUVEL_DEFAULT_ASAN OFF)
    set(FLUVEL_DEFAULT_TSAN OFF)
else()
    set(FLUVEL_DEFAULT_CCACHE ${FLUVEL_LOCAL_DEFAULT_CCACHE})
    set(FLUVEL_DEFAULT_UNITY ${FLUVEL_LOCAL_DEFAULT_UNITY})
    set(FLUVEL_DEFAULT_ASAN ${FLUVEL_LOCAL_DEFAULT_ASAN})
    set(FLUVEL_DEFAULT_TSAN ${FLUVEL_LOCAL_DEFAULT_TSAN})
endif()

option(
    ENABLE_CCACHE_BUILD
    "Enable ccache support"
    ${FLUVEL_DEFAULT_CCACHE}
)

option(
    ENABLE_UNITY_BUILD
    "Enable unity build"
    ${FLUVEL_DEFAULT_UNITY}
)

option(
    ENABLE_ASAN
    "Enable Sanitizers"
    ${FLUVEL_DEFAULT_ASAN}
)

option(
    ENABLE_TSAN
    "Enable ThreadSanitizer"
    ${FLUVEL_DEFAULT_TSAN}
)

if(ENABLE_CCACHE_BUILD)

    find_program(CCACHE_PROGRAM ccache)

    if(CCACHE_PROGRAM)

        message(STATUS "ccache found: ${CCACHE_PROGRAM}")

        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})

    else()

        message(STATUS "ccache not found")

    endif()

endif()

if(ENABLE_UNITY_BUILD)
    set(CMAKE_UNITY_BUILD ON)
endif()

if(ENABLE_ASAN)
    message(STATUS "Sanitizers ENABLED")

    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=undefined
        -g

        -Wconversion
        -Wsign-conversion

        -Wcast-qual
        -Wformat=2
        -Wundef
        -Wimplicit-fallthrough
        -Wmissing-field-initializers

        #-Weverything
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(
            -Wconditional-uninitialized
        )
    endif()

    add_link_options(
        -fsanitize=address,undefined
    )
endif()

if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")

    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif()
