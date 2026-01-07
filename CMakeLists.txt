cmake_minimum_required(VERSION 3.16)
project(Ofeli VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR})

set(OFELI_ROOT_DIR
    ${PROJECT_SOURCE_DIR}
    CACHE PATH "Ofeli root directory"
)

set(OFELI_RESOURCES_DIR
    ${OFELI_ROOT_DIR}/resources
)

set(OFELI_TRANSLATIONS_DIR
    ${OFELI_ROOT_DIR}/translations
)

set(OFELI_BUILD_DIR
    ${OFELI_ROOT_DIR}/build
)

set(CMAKE_BINARY_DIR ${OFELI_BUILD_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia MultimediaWidgets Widgets LinguistTools)

add_subdirectory(src/app)
add_subdirectory(src/image_processing)

set(CPACK_PACKAGE_NAME "Ofeli")
set(CPACK_PACKAGE_VERSION "2.0.0")
set(CPACK_GENERATOR "TGZ;DEB;RPM")

include(CPack)
