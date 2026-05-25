# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

# =========================================================
# Fluvel Packaging
# =========================================================

# ---------------------------------------------------------
# Common package metadata
# ---------------------------------------------------------

set(CPACK_PACKAGE_NAME "fluvel")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Real-time computer vision and image analysis for research"
)

set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
    "Research-oriented image segmentation application using active contour and level-set methods."
)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER
    "Fabien Bessy <fabien.bessy@gmail.com>"
)

# =========================================================
# Linux
# =========================================================

if(FLUVEL_PLATFORM_LINUX)

    include(GNUInstallDirs)

    set(CPACK_GENERATOR "DEB")

    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libqt6core6,
         libqt6gui6,
         libqt6widgets6,
         libqt6multimedia6,
         libqt6svg6,
         qt6-image-formats-plugins"
    )

    # =====================================================
    # Linux install directories (FHS)
    # =====================================================

    set(LINUX_INSTALL_DESKTOPDIR
        ${CMAKE_INSTALL_DATADIR}/applications
    )

    set(LINUX_INSTALL_METAINFO_DIR
        ${CMAKE_INSTALL_DATADIR}/metainfo
    )

    set(LINUX_INSTALL_ICONS_DIR
        ${CMAKE_INSTALL_DATADIR}/icons/hicolor
    )

    # =====================================================
    # Executable
    # =====================================================

    install(TARGETS ${APP_TARGET_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # =====================================================
    # Desktop entry
    # =====================================================

    install(FILES
        ${FLUVEL_PACKAGING_DIR}/linux/org.fluvel.Fluvel.desktop
        DESTINATION ${LINUX_INSTALL_DESKTOPDIR}
    )

    # =====================================================
    # AppStream metadata
    # =====================================================

    string(TIMESTAMP FLUVEL_RELEASE_DATE "%Y-%m-%d")

    configure_file(
        ${FLUVEL_PACKAGING_DIR}/linux/org.fluvel.Fluvel.metainfo.xml.in
        ${CMAKE_CURRENT_BINARY_DIR}/org.fluvel.Fluvel.metainfo.xml
        @ONLY
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/org.fluvel.Fluvel.metainfo.xml
        DESTINATION ${LINUX_INSTALL_METAINFO_DIR}
    )

    # =====================================================
    # Scalable SVG icon
    # =====================================================

    install(FILES
        ${FLUVEL_PACKAGING_DIR}/linux/org.fluvel.Fluvel.svg
        DESTINATION ${LINUX_INSTALL_ICONS_DIR}/scalable/apps
    )

    # =====================================================
    # PNG icons
    # =====================================================

    install(FILES
        ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-32.png
        DESTINATION ${LINUX_INSTALL_ICONS_DIR}/32x32/apps
        RENAME org.fluvel.Fluvel.png
    )

    install(FILES
        ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-48.png
        DESTINATION ${LINUX_INSTALL_ICONS_DIR}/48x48/apps
        RENAME org.fluvel.Fluvel.png
    )

    install(FILES
        ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-64.png
        DESTINATION ${LINUX_INSTALL_ICONS_DIR}/64x64/apps
        RENAME org.fluvel.Fluvel.png
    )

    install(FILES
        ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-128.png
        DESTINATION ${LINUX_INSTALL_ICONS_DIR}/128x128/apps
        RENAME org.fluvel.Fluvel.png
    )

    install(FILES
        ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-256.png
        DESTINATION ${LINUX_INSTALL_ICONS_DIR}/256x256/apps
        RENAME org.fluvel.Fluvel.png
    )

endif()

# =========================================================
# Windows
# =========================================================

if(FLUVEL_PLATFORM_WINDOWS)

    set(app_icon_resource_windows_in
        ${FLUVEL_RESOURCES_DIR}/platforms/windows/Fluvel.rc.in
    )

    set(FLUVEL_VERSION ${PROJECT_VERSION})

    string(REPLACE "." "," FLUVEL_VERSION_COMMA "${PROJECT_VERSION},0")

    set(FLUVEL_ICON_PATH
        "${FLUVEL_RESOURCES_DIR}/platforms/windows/Fluvel.ico"
    )

    if(DEFINED FLUVEL_GIT_COMMIT)
        string(SUBSTRING "${FLUVEL_GIT_COMMIT}" 0 7
               FLUVEL_GIT_COMMIT_SHORT)
    else()
        set(FLUVEL_GIT_COMMIT_SHORT "unknown")
    endif()

    file(TO_CMAKE_PATH "${FLUVEL_ICON_PATH}" FLUVEL_ICON_PATH)

    configure_file(
        ${app_icon_resource_windows_in}
        ${CMAKE_CURRENT_BINARY_DIR}/Fluvel.rc
        @ONLY
    )

    target_sources(${APP_TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/Fluvel.rc
    )

    set_target_properties(${APP_TARGET_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

endif()

# =========================================================
# macOS
# =========================================================

if(FLUVEL_PLATFORM_MACOS)

    if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
    endif()

    set(MACOSX_BUNDLE_ICON_FILE Fluvel.icns)

    set(app_icon_macos
        ${FLUVEL_PACKAGING_DIR}/macos/Fluvel.icns
    )

    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(${APP_TARGET_NAME} PRIVATE ${app_icon_macos})

    set_target_properties(${APP_TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Fluvel"
    )

    set(FLUVEL_VERSION ${PROJECT_VERSION})

    configure_file(
        ${FLUVEL_PACKAGING_DIR}/macos/Info.plist.in
        ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        @ONLY
    )

    set_target_properties(${APP_TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
    )

    message(STATUS "Using plist: ${CMAKE_CURRENT_BINARY_DIR}/Info.plist")

    install(TARGETS ${APP_TARGET_NAME}
        BUNDLE DESTINATION .
    )

endif()

# =========================================================
# Android
# =========================================================

if(FLUVEL_PLATFORM_ANDROID)

    message(STATUS "Android packaging handled by Qt")

    # --- Android packaging ---
    set_target_properties(${APP_TARGET_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR ${FLUVEL_PACKAGING_DIR}/android

        # --- Options possibles ---
        # QT_ANDROID_APP_NAME ${APP_TARGET_NAME}
        # QT_ANDROID_APP_LIB_NAME ${APP_TARGET_NAME}
        # QT_ANDROID_PACKAGE_NAME "org.fluvel.app"
        # QT_ANDROID_GUI TRUE
        # QT_ANDROID_DEPLOYMENT_SETTINGS_FILE
        # ${CMAKE_CURRENT_BINARY_DIR}/android-${APP_TARGET_NAME}-deployment-settings.json
        # OUTPUT_NAME "fluvel"
    )

endif()

# =========================================================
# CPack
# =========================================================

if(FLUVEL_PLATFORM_LINUX)
    include(CPack)
endif()