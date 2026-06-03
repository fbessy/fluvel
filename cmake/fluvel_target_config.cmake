# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

function(fluvel_configure_target target)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

        target_compile_options(${target} PRIVATE

            $<$<CONFIG:Debug>:
                -O0
                -g

                -Wall
                -Wextra
                -Wpedantic

                -Wshadow
                -Wuninitialized

                -Wnon-virtual-dtor
                -Woverloaded-virtual

                -Wold-style-cast

                -Wfloat-equal

                -Wnull-dereference
            >

            $<$<CONFIG:Release>:
                -O3
                -DNDEBUG
            >

            $<$<CONFIG:RelWithDebInfo>:
                -O2
                -g
                -DNDEBUG
            >

            $<$<CONFIG:MinSizeRel>:
                -Os
                -DNDEBUG
            >
        )

    elseif(MSVC)

        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:/Od /Zi>
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
            $<$<CONFIG:MinSizeRel>:/O1>
        )

    endif()

endfunction()

function(fluvel_configure_standalone_target target)

    fluvel_configure_target(${target})

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

        target_compile_options(${target} PRIVATE

            $<$<CONFIG:Debug>:
                -Wswitch-enum
            >
        )

    endif()

endfunction()
