set(target_name Ofeli)

# 34 files
set(app_src
    "main.cpp"
    "main_window.cpp" "main_window.hpp"
    "settings_window.cpp" "settings_window.hpp"

    "evaluation_window.cpp" "evaluation_window.hpp"
    "evaluation_widget.cpp" "evaluation_widget.hpp"
    "pixmap_widget.cpp" "pixmap_widget.hpp"
    "qspinbox_kernel.cpp" "qspinbox_kernel.hpp"
    "scroll_area_widget.cpp" "scroll_area_widget.hpp"

    "camera_window.cpp" "camera_window.hpp"
    "video_active_contour_thread.cpp" "video_active_contour_thread.hpp"
    "camera_graphics_view.cpp" "camera_graphics_view.hpp"
    "frame_stats.cpp" "frame_stats.hpp"
    "frame_clock.cpp" "frame_clock.hpp"

    "language_window.cpp" "language_window.hpp"
    "about_window.cpp" "about_window.hpp"

    "contour_rendering.cpp" "contour_rendering.hpp"

    "application_settings.cpp" "application_settings.hpp"
    "runtime_settings.cpp" "runtime_settings.hpp"
    "image_adapters.hpp"
    "common_settings.hpp"
)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE Ofeli.icns)

    set(app_icon_macos
        ${OFELI_RESOURCES_DIR}/icons/app/Ofeli.icns
    )

    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    qt_add_executable(${target_name}
        MACOSX_BUNDLE
        ${app_src}
        ${app_icon_macos}
    )

elseif(WIN32)
    set(app_icon_resource_windows
        ${OFELI_RESOURCES_DIR}/icons/app/Ofeli.rc
    )

    qt_add_executable(${target_name}
        WIN32
        ${app_src}
        ${app_icon_resource_windows}
    )

else()
    qt_add_executable(${target_name}
        ${app_src}
    )
endif()

# --- TS ---
file(GLOB TS_FILES
    "${OFELI_TRANSLATIONS_DIR}/*.ts"
)

# --- TS -> QM ---
qt6_add_translation(QM_FILES ${TS_FILES})

# --- Alias obligatoires ---
foreach(qm ${QM_FILES})
    get_filename_component(qm_name ${qm} NAME)
    set_source_files_properties(${qm}
        PROPERTIES QT_RESOURCE_ALIAS "${qm_name}"
    )
endforeach()

# --- Resources ---
qt_add_resources(${target_name} ofeli_translations
    PREFIX "/i18n"
    FILES ${QM_FILES}
)

set(APP_RESOURCES
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli.svg
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli.png

    #${OFELI_RESOURCES_DIR}/icons/toolbar/application-exit.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/configure.svg
    #${OFELI_RESOURCES_DIR}/icons/toolbar/document-open.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/document-open-recent.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/document-print.svg
    #${OFELI_RESOURCES_DIR}/icons/toolbar/document-save-as.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/edit-clear-list.svg
    #${OFELI_RESOURCES_DIR}/icons/toolbar/help-about.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/help-contents.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/insert-horizontal-rule.svg
    #${OFELI_RESOURCES_DIR}/icons/toolbar/media-playback-start.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/preferences-desktop-locale.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/zoom-in.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/zoom-original.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/zoom-out.svg

    ${OFELI_RESOURCES_DIR}/docs/application_to_plant_recognition.pdf

    ${OFELI_RESOURCES_DIR}/licenses/license_en.txt
    ${OFELI_RESOURCES_DIR}/licenses/license_fr.txt
)

foreach(res ${APP_RESOURCES})
    file(RELATIVE_PATH rel_path ${OFELI_RESOURCES_DIR} ${res})
    set_source_files_properties(
        ${res}
        PROPERTIES QT_RESOURCE_ALIAS ${rel_path}
    )
endforeach()

qt_add_resources(${target_name} app_resources
    PREFIX "/"
    FILES ${APP_RESOURCES}
)

if(APPLE AND NOT IOS)
    set_target_properties(${target_name} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${OFELI_RESOURCES_DIR}/platforms/macos/Info.plist.in"
    )
endif()

target_link_libraries(${target_name} PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::MultimediaWidgets
    Qt::Widgets
    Qt::PrintSupport
    image_processing
)

if(OpenCV_FOUND)
    target_compile_definitions(${target_name} PRIVATE OFELI_HAS_OPEN_CV)
endif()

if(Qt6_FOUND)
    target_compile_definitions(${target_name} PRIVATE OFELI_HAS_QT)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "(Clang|GNU)")
    target_compile_options(${target_name} PRIVATE
        -O3
        -Wall
        -Wextra
        -Wpedantic
        # -Werror  <-- je te conseille de l'enlever
    )
endif()

# install(TARGETS ${target_name}
#         RUNTIME DESTINATION "${OFELI_BUILD_DIR}/runtime"
#         BUNDLE DESTINATION  "${OFELI_BUILD_DIR}/bundle"
#         LIBRARY DESTINATION "${OFELI_BUILD_DIR}/library"
# )
