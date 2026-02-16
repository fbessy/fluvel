cmake_minimum_required(VERSION 3.16)

project(Ofeli VERSION 2.0.0 LANGUAGES CXX)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer ENABLED")

    add_compile_options(
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
    )

    add_link_options(
        -fsanitize=address
    )
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(OFELI_ROOT_DIR
    ${PROJECT_SOURCE_DIR}
)

set(OFELI_RESOURCES_DIR
    ${OFELI_ROOT_DIR}/resources
)

set(OFELI_TRANSLATIONS_DIR
    ${OFELI_ROOT_DIR}/translations
)

set(QT_COMPONENTS
    Core Gui Multimedia MultimediaWidgets Widgets LinguistTools
)

if(ANDROID OR IOS)
    list(APPEND QT_COMPONENTS Qml Quick)
endif()

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

add_subdirectory(src/image_processing)
add_subdirectory(src/app)

include(CPack)
