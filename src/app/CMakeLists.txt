set(target_name Ofeli)

set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(UI_WIN_DIR ${APP_DIR}/ui/windows)
set(UI_COMP_DIR ${APP_DIR}/ui/components)
set(CORE_DIR ${APP_DIR}/core)
set(INTEROP_DIR ${APP_DIR}/interop)

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

    ${UI_WIN_DIR}/camera_settings_window.cpp
    ${UI_WIN_DIR}/camera_settings_window.hpp

    ${UI_WIN_DIR}/language_window.cpp
    ${UI_WIN_DIR}/language_window.hpp

    ${UI_WIN_DIR}/about_window.cpp
    ${UI_WIN_DIR}/about_window.hpp

    ${UI_COMP_DIR}/algo_settings_widget.cpp
    ${UI_COMP_DIR}/algo_settings_widget.hpp

    ${UI_COMP_DIR}/display_settings_widget.cpp
    ${UI_COMP_DIR}/display_settings_widget.hpp

    ${UI_COMP_DIR}/analysis_widget.cpp
    ${UI_COMP_DIR}/analysis_widget.hpp

    ${UI_COMP_DIR}/color_selector_widget.cpp
    ${UI_COMP_DIR}/color_selector_widget.hpp

    ${UI_COMP_DIR}/kernel_size_spinbox.cpp
    ${UI_COMP_DIR}/kernel_size_spinbox.hpp

    ${UI_COMP_DIR}/icon_loader.cpp
    ${UI_COMP_DIR}/icon_loader.hpp

    ${UI_COMP_DIR}/image_view.cpp
    ${UI_COMP_DIR}/image_view.hpp

    ${UI_COMP_DIR}/image_view_interaction.hpp
    ${UI_COMP_DIR}/interaction_set.cpp
    ${UI_COMP_DIR}/interaction_set.hpp

    ${UI_COMP_DIR}/view_behavior.hpp

    ${UI_COMP_DIR}/fullscreen_behavior.cpp
    ${UI_COMP_DIR}/fullscreen_behavior.hpp

    ${UI_COMP_DIR}/autofit_behavior.cpp
    ${UI_COMP_DIR}/autofit_behavior.hpp

    ${UI_COMP_DIR}/pan_behavior.cpp
    ${UI_COMP_DIR}/pan_behavior.hpp

    ${UI_COMP_DIR}/zoom_behavior.cpp
    ${UI_COMP_DIR}/zoom_behavior.hpp

    ${UI_COMP_DIR}/color_picker_behavior.cpp
    ${UI_COMP_DIR}/color_picker_behavior.hpp

    ${UI_COMP_DIR}/pixel_info_behavior.cpp
    ${UI_COMP_DIR}/pixel_info_behavior.hpp

    ${UI_COMP_DIR}/drag_drop_behavior.cpp
    ${UI_COMP_DIR}/drag_drop_behavior.hpp

    ${UI_COMP_DIR}/pixel_info_overlay.cpp
    ${UI_COMP_DIR}/pixel_info_overlay.hpp

    ${UI_COMP_DIR}/contour_point_item.cpp
    ${UI_COMP_DIR}/contour_point_item.hpp

    ${UI_COMP_DIR}/frame_stats_view.cpp
    ${UI_COMP_DIR}/frame_stats_view.hpp

    ${UI_COMP_DIR}/phi_view_model.cpp
    ${UI_COMP_DIR}/phi_view_model.hpp

    ${UI_COMP_DIR}/contour_rendering_qimage.cpp
    ${UI_COMP_DIR}/contour_rendering_qimage.hpp

    ${UI_COMP_DIR}/overlay_text_item.cpp
    ${UI_COMP_DIR}/overlay_text_item.hpp

    ${UI_COMP_DIR}/right_panel_toggle_button.cpp
    ${UI_COMP_DIR}/right_panel_toggle_button.hpp

    ${UI_COMP_DIR}/image_view_listener.hpp

    ${CORE_DIR}/image_controller.cpp
    ${CORE_DIR}/image_controller.hpp

    ${CORE_DIR}/image_settings_controller.cpp
    ${CORE_DIR}/image_settings_controller.hpp

    ${CORE_DIR}/camera_controller.cpp
    ${CORE_DIR}/camera_controller.hpp

    ${CORE_DIR}/active_contour_worker.cpp
    ${CORE_DIR}/active_contour_worker.hpp

    ${CORE_DIR}/video_active_contour_thread.cpp
    ${CORE_DIR}/video_active_contour_thread.hpp

    ${CORE_DIR}/frame_clock.cpp
    ${CORE_DIR}/frame_clock.hpp

    ${CORE_DIR}/application_settings.cpp
    ${CORE_DIR}/application_settings.hpp

    ${CORE_DIR}/common_settings.hpp

    ${CORE_DIR}/phi_editor.cpp
    ${CORE_DIR}/phi_editor.hpp

    ${CORE_DIR}/shape_overlay_renderer.cpp
    ${CORE_DIR}/shape_overlay_renderer.hpp

    ${CORE_DIR}/shape_type.hpp
    ${CORE_DIR}/camera_stats.hpp

    ${INTEROP_DIR}/image_adapters.hpp
    ${INTEROP_DIR}/color_adapters.hpp
    ${INTEROP_DIR}/contour_adapters.hpp
)

qt_add_executable(${target_name}
    ${app_src}
    resources.qrc)

if(ANDROID)
    # qt_add_qml_module(${target_name}
    #     URI ofeli
    #     VERSION 1.0
    #     QML_FILES
    #         qml/Main.qml
    # )
    qt_add_resources(${target_name} "qml"
        PREFIX "/"
        FILES
            qml/Main.qml
            qml/lenna.bmp
    )
endif()

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE Ofeli.icns)

    set(app_icon_macos
        ${OFELI_RESOURCES_DIR}/icons/app/Ofeli.icns
    )

    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(${target_name} PRIVATE ${app_icon_macos})

    set_target_properties(${target_name} PROPERTIES
        MACOSX_BUNDLE TRUE
    )

elseif(WIN32)
    set(app_icon_resource_windows
        ${OFELI_RESOURCES_DIR}/icons/app/Ofeli.rc
    )

    target_sources(${target_name} PRIVATE
        ${app_icon_resource_windows}
    )

    set_target_properties(${target_name} PROPERTIES
        WIN32_EXECUTABLE TRUE
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


    ${OFELI_RESOURCES_DIR}/icons/toolbar/application-exit-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/right_panel_off.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/right_panel_on.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/camera-web-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/configure-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/document-open-recent-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/document-open-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/document-save-as-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/edit-clear-history.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/go-next-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/help-about-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/measure-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/media-playback-pause-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/media-playback-start-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/media-playback-stop-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/media-playlist-repeat-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/media-seek-forward-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/preferences-desktop-locale-symbolic.svg
    ${OFELI_RESOURCES_DIR}/icons/toolbar/view-preview-symbolic.svg

    ${OFELI_RESOURCES_DIR}/docs/application_to_plant_recognition.pdf

    ${OFELI_RESOURCES_DIR}/licenses/Licence_CeCILL_V2.1-en.txt
    ${OFELI_RESOURCES_DIR}/licenses/Licence_CeCILL_V2.1-fr.txt
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
    ${INTEROP_DIR}
)

if(OpenCV_FOUND)
    target_compile_definitions(${target_name} PRIVATE OFELI_USE_OPENCV)
endif()

if(Qt6_FOUND)
    target_compile_definitions(${target_name} PRIVATE OFELI_USE_QT)
endif()


if(ENABLE_ASAN)
    target_compile_definitions(${target_name} PRIVATE OFELI_ASAN)
else()
    target_compile_definitions(${target_name} PRIVATE
        $<$<CONFIG:Debug>:OFELI_DEBUG>
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:
            -O0
            -g
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
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
elseif (MSVC)
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
        $<$<CONFIG:MinSizeRel>:/O1>
    )
endif()

set(APP_QT_LIBS
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Widgets
)

if(ANDROID)
    list(APPEND APP_QT_LIBS
        Qt6::Qml
        Qt6::Quick
    )
endif()

target_link_libraries(${target_name} PRIVATE
    ${APP_QT_LIBS}
    ofeli::image_processing
)

target_compile_definitions(Ofeli PRIVATE
    OFELI_CMAKE_VERSION="${CMAKE_VERSION}"
    OFELI_CMAKE_GENERATOR="${CMAKE_GENERATOR}"
    OFELI_CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

install(TARGETS ${target_name}
        RUNTIME DESTINATION bin)

if(UNIX AND NOT APPLE)
install(FILES ${CMAKE_SOURCE_DIR}/resources/platforms/linux/org.ofeli.App.desktop
        DESTINATION share/applications)

install(FILES ${CMAKE_SOURCE_DIR}/resources/icons/app/org.ofeli.App.svg
        DESTINATION share/icons/hicolor/scalable/apps)
endif()

# target_compile_features(my_target PUBLIC cxx_std_23) au lieu de global dans le fichier projet
# définir par projets chaque prérequis

#idem préférer :
# set_target_properties(my_qt_target PROPERTIES
#     AUTOMOC ON
#     AUTOUIC ON
#     AUTORCC ON
# )
