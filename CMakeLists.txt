cmake_minimum_required(VERSION 3.16)

project(Fluvel VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

# =========================
# Local development defaults
# Modify here if needed
# =========================

set(FLUVEL_LOCAL_DEFAULT_CCACHE OFF)
set(FLUVEL_LOCAL_DEFAULT_UNITY OFF)

set(FLUVEL_LOCAL_DEFAULT_ASAN OFF)
set(FLUVEL_LOCAL_DEFAULT_TSAN OFF)

include(cmake/fluvel_platforms.cmake)
include(cmake/fluvel_build_config.cmake)
include(cmake/fluvel_qt.cmake)

set(CMAKE_AUTOMOC_VERBOSE ON)

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

# Linux install directories (FHS)
set(LINUX_INSTALL_BINDIR bin)
set(LINUX_INSTALL_LIBDIR lib)
set(LINUX_INSTALL_DATADIR share)

set(LINUX_INSTALL_DESKTOPDIR
    ${LINUX_INSTALL_DATADIR}/applications)

set(LINUX_INSTALL_ICONDIR
    ${LINUX_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

# =========================
# Experimental UI options
# =========================

option(FLUVEL_USE_QML
       "Enable experimental QML UI"
       OFF)

option(FLUVEL_UI_DESKTOP
       "Use QWidget UI on Mobile"
       ON)

add_subdirectory(src/image_processing)
add_subdirectory(src/app)

include(CPack)
