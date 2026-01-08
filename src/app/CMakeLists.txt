set(target_name Ofeli)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(UI_WIN_DIR ${APP_DIR}/ui/windows)
set(UI_COMP_DIR ${APP_DIR}/ui/components)
set(CORE_DIR ${APP_DIR}/core)

set(app_src

    ${APP_DIR}/main.cpp

    ${UI_WIN_DIR}/image_window.cpp
    ${UI_WIN_DIR}/image_window.hpp

    ${UI_WIN_DIR}/camera_window.cpp
    ${UI_WIN_DIR}/camera_window.hpp

    ${UI_WIN_DIR}/analysis_window.cpp
    ${UI_WIN_DIR}/analysis_window.hpp

    ${UI_WIN_DIR}/settings_window.cpp
    ${UI_WIN_DIR}/settings_window.hpp

    ${UI_WIN_DIR}/language_window.cpp
    ${UI_WIN_DIR}/language_window.hpp

    ${UI_WIN_DIR}/about_window.cpp
    ${UI_WIN_DIR}/about_window.hpp

    ${UI_COMP_DIR}/image_view.cpp
    ${UI_COMP_DIR}/image_view.hpp

    ${UI_COMP_DIR}/camera_overlay_widget.cpp
    ${UI_COMP_DIR}/camera_overlay_widget.hpp

    ${UI_COMP_DIR}/analysis_widget.cpp
    ${UI_COMP_DIR}/analysis_widget.hpp

    ${UI_COMP_DIR}/pixmap_widget.cpp
    ${UI_COMP_DIR}/pixmap_widget.hpp

    ${UI_COMP_DIR}/kernel_size_spinbox.cpp
    ${UI_COMP_DIR}/kernel_size_spinbox.hpp

    ${UI_COMP_DIR}/scroll_area_widget.cpp
    ${UI_COMP_DIR}/scroll_area_widget.hpp

    ${UI_COMP_DIR}/frame_stats_view.cpp
    ${UI_COMP_DIR}/frame_stats_view.hpp

    ${CORE_DIR}/image_controller.cpp
    ${CORE_DIR}/image_controller.hpp

    ${CORE_DIR}/active_contour_worker.cpp
    ${CORE_DIR}/active_contour_worker.hpp

    ${CORE_DIR}/video_active_contour_thread.cpp
    ${CORE_DIR}/video_active_contour_thread.hpp

    ${CORE_DIR}/frame_clock.cpp
    ${CORE_DIR}/frame_clock.hpp

    ${CORE_DIR}/contour_rendering.cpp
    ${CORE_DIR}/contour_rendering.hpp

    ${CORE_DIR}/application_settings.cpp
    ${CORE_DIR}/application_settings.hpp

    ${CORE_DIR}/runtime_settings.hpp
    ${CORE_DIR}/image_adapters.hpp
    ${CORE_DIR}/common_settings.hpp
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
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli_16.png
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli_32.png
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli_48.png
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli_128.png
    ${OFELI_RESOURCES_DIR}/icons/app/Ofeli_256.png

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

target_include_directories(${target_name} PRIVATE
    ${CORE_DIR}
    ${UI_COMP_DIR}
    ${UI_WIN_DIR}
)

if(OpenCV_FOUND)
    target_compile_definitions(${target_name} PRIVATE OFELI_HAS_OPEN_CV)
endif()

if(Qt6_FOUND)
    target_compile_definitions(${target_name} PRIVATE OFELI_HAS_QT)
endif()

target_compile_definitions(${target_name} PRIVATE
    $<$<CONFIG:Debug>:OFELI_DEBUG>
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:
            -O0
            -g
            -Wall
            -Wextra
            -Wpedantic
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
elseif (MSVC)
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
        $<$<CONFIG:MinSizeRel>:/O1>
    )
endif()

target_link_libraries(${target_name} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::MultimediaWidgets
    Qt::Widgets
    ofeli::image_processing
)

# target_compile_features(my_target PUBLIC cxx_std_23) au lieu de global dans le fichier projet
# définir par projets chaque prérequis

#idem préférer :
# set_target_properties(my_qt_target PROPERTIES
#     AUTOMOC ON
#     AUTOUIC ON
#     AUTORCC ON
# )
