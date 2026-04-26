cmake_minimum_required(VERSION 3.16)

project(Fluvel VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

option(FLUVEL_UI_DESKTOP "Use QWidget UI on Mobile" ON)
option(ENABLE_UNITY_BUILD "Enable unity build" OFF)
option(ENABLE_ASAN "Enable Sanitizers" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC_VERBOSE ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(FLUVEL_ROOT_DIR
    ${PROJECT_SOURCE_DIR}
)

set(FLUVEL_RESOURCES_DIR
    ${FLUVEL_ROOT_DIR}/resources
)

set(FLUVEL_TRANSLATIONS_DIR
    ${FLUVEL_ROOT_DIR}/translations
)

set(QT_COMPONENTS
    Core Gui Multimedia MultimediaWidgets Widgets LinguistTools
)

if((ANDROID OR IOS) AND NOT FLUVEL_UI_DESKTOP)
    list(APPEND QT_COMPONENTS Qml Quick)
endif()

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

if (COMMAND qt_policy)
    qt_policy(SET QTP0002 NEW)
endif()

add_subdirectory(src/image_processing)
add_subdirectory(src/app)

include(CPack)
