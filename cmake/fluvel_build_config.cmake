set(FLUVEL_IS_CI FALSE)

if(DEFINED ENV{CI})
    set(FLUVEL_IS_CI TRUE)
endif()

if(FLUVEL_IS_CI)

    if(NOT DEFINED ENABLE_CCACHE_BUILD)
        set(ENABLE_CCACHE_BUILD OFF)
    endif()

    if(NOT DEFINED ENABLE_UNITY_BUILD)
        set(ENABLE_UNITY_BUILD OFF)
    endif()

    if(NOT DEFINED ENABLE_ASAN)
        set(ENABLE_ASAN OFF)
    endif()

    if(NOT DEFINED ENABLE_TSAN)
        set(ENABLE_TSAN OFF)
    endif()

else()

    if(NOT DEFINED ENABLE_CCACHE_BUILD)
        set(ENABLE_CCACHE_BUILD ${FLUVEL_LOCAL_DEFAULT_CCACHE})
    endif()

    if(NOT DEFINED ENABLE_UNITY_BUILD)
        set(ENABLE_UNITY_BUILD ${FLUVEL_LOCAL_DEFAULT_UNITY})
    endif()

    if(NOT DEFINED ENABLE_ASAN)
        set(ENABLE_ASAN ${FLUVEL_LOCAL_DEFAULT_ASAN})
    endif()

    if(NOT DEFINED ENABLE_TSAN)
        set(ENABLE_TSAN ${FLUVEL_LOCAL_DEFAULT_TSAN})
    endif()

endif()

option(ENABLE_CCACHE_BUILD "Enable ccache support" ${ENABLE_CCACHE_BUILD})
option(ENABLE_UNITY_BUILD "Enable unity build" ${ENABLE_UNITY_BUILD})

option(ENABLE_ASAN "Enable Sanitizers" ${ENABLE_ASAN})
option(ENABLE_TSAN "Enable ThreadSanitizer" ${ENABLE_TSAN})

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
    )

    add_link_options(
        -fsanitize=address,undefined
    )
endif()

if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")

    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif()
