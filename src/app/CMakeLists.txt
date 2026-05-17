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

    ${UI_COMP_DIR}/image_viewer_widget.cpp
    ${UI_COMP_DIR}/image_viewer_widget.hpp

    ${UI_COMP_DIR}/image_viewer_interaction.hpp
    ${UI_COMP_DIR}/interaction_set.cpp
    ${UI_COMP_DIR}/interaction_set.hpp

    ${UI_COMP_DIR}/image_viewer_behavior.hpp

    ${UI_COMP_DIR}/fullscreen_behavior.cpp
    ${UI_COMP_DIR}/fullscreen_behavior.hpp

    ${UI_COMP_DIR}/autofit_behavior.cpp
    ${UI_COMP_DIR}/autofit_behavior.hpp

    ${UI_COMP_DIR}/pan_behavior.cpp
    ${UI_COMP_DIR}/pan_behavior.hpp

    ${UI_COMP_DIR}/color_picker_behavior.cpp
    ${UI_COMP_DIR}/color_picker_behavior.hpp

    ${UI_COMP_DIR}/pixel_info_behavior.cpp
    ${UI_COMP_DIR}/pixel_info_behavior.hpp

    ${UI_COMP_DIR}/drag_drop_behavior.cpp
    ${UI_COMP_DIR}/drag_drop_behavior.hpp

    ${UI_COMP_DIR}/initialization_behavior.cpp
    ${UI_COMP_DIR}/initialization_behavior.hpp

    ${UI_COMP_DIR}/pixel_info_overlay.cpp
    ${UI_COMP_DIR}/pixel_info_overlay.hpp

    ${UI_COMP_DIR}/contour_point_item.cpp
    ${UI_COMP_DIR}/contour_point_item.hpp

    ${UI_COMP_DIR}/frame_stats_view.cpp
    ${UI_COMP_DIR}/frame_stats_view.hpp

    ${UI_COMP_DIR}/phi_view_model.cpp
    ${UI_COMP_DIR}/phi_view_model.hpp

    ${UI_COMP_DIR}/overlay_text_item.cpp
    ${UI_COMP_DIR}/overlay_text_item.hpp

    ${UI_COMP_DIR}/zoom_overlay_controller.cpp
    ${UI_COMP_DIR}/zoom_overlay_controller.hpp

    ${UI_COMP_DIR}/right_panel_toggle_button.cpp
    ${UI_COMP_DIR}/right_panel_toggle_button.hpp

    ${UI_COMP_DIR}/image_viewer_listener.hpp

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

    ${CORE_DIR}/frame_data.hpp

    ${CORE_DIR}/frame_clock.cpp
    ${CORE_DIR}/frame_clock.hpp

    ${CORE_DIR}/application_settings.cpp
    ${CORE_DIR}/application_settings.hpp

    ${CORE_DIR}/application_settings_types.hpp

    ${CORE_DIR}/phi_editor.cpp
    ${CORE_DIR}/phi_editor.hpp

    ${CORE_DIR}/shape_type.hpp
    ${CORE_DIR}/camera_stats.hpp

    ${CORE_DIR}/image_debug.cpp
    ${CORE_DIR}/image_debug.hpp

    ${CORE_DIR}/qimage_utils.cpp
    ${CORE_DIR}/qimage_utils.hpp

    ${CORE_DIR}/file_utils.cpp
    ${CORE_DIR}/file_utils.hpp

    ${CORE_DIR}/qcolor_utils.cpp
    ${CORE_DIR}/qcolor_utils.hpp

    ${CORE_DIR}/device_id_utils.hpp
    ${CORE_DIR}/camera_format_utils.hpp

    ${INTEROP_DIR}/image_adapters.hpp
    ${INTEROP_DIR}/color_adapters.hpp
    ${INTEROP_DIR}/contour_adapters.hpp
)

qt_add_executable(${FLUVEL_TARGET_NAME}
    ${app_src})

if(FLUVEL_PLATFORM_LINUX)
    set(FLUVEL_OUTPUT_NAME "fluvel")
else()
    set(FLUVEL_OUTPUT_NAME "Fluvel")
endif()

set_target_properties(${FLUVEL_TARGET_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    OUTPUT_NAME ${FLUVEL_OUTPUT_NAME}
)

#if(FLUVEL_PLATFORM_ANDROID)
    # qt_add_qml_module(${FLUVEL_TARGET_NAME}
    #     URI fluvel
    #     VERSION 1.0
    #     QML_FILES
    #         qml/Main.qml
    # )
    # qt_add_resources(${FLUVEL_TARGET_NAME} "qml"
    #     PREFIX "/"
    #     FILES
    #         qml/Main.qml
    #         qml/lenna.bmp
    # )
#endif()

# --- TS ---
file(GLOB TS_FILES
    "${FLUVEL_TRANSLATIONS_DIR}/*.ts"
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
qt_add_resources(${FLUVEL_TARGET_NAME} fluvel_translations
    PREFIX "/i18n"
    FILES ${QM_FILES}
)

set(APP_RESOURCES
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel.svg
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-16.png
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-22.png
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-32.png
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-48.png
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-64.png
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-128.png
    ${FLUVEL_RESOURCES_DIR}/icons/app/fluvel-256.png


    ${FLUVEL_RESOURCES_DIR}/icons/actions/image-session-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/actions/camera-web-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/actions/application-exit-symbolic.svg

    ${FLUVEL_RESOURCES_DIR}/icons/actions/edit-clear-history.svg

    ${FLUVEL_RESOURCES_DIR}/icons/actions/measure-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/actions/settings-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/actions/language-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/actions/help-about-symbolic.svg

    ${FLUVEL_RESOURCES_DIR}/icons/actions/go-next-symbolic.svg


    ${FLUVEL_RESOURCES_DIR}/icons/file/document-open-recent-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/file/document-open-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/file/document-save-as-symbolic.svg

    ${FLUVEL_RESOURCES_DIR}/icons/media/media-playback-pause-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/media/media-playback-start-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/media/media-playback-stop-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/media/media-playlist-repeat-symbolic.svg
    ${FLUVEL_RESOURCES_DIR}/icons/media/media-seek-forward-symbolic.svg

    ${FLUVEL_RESOURCES_DIR}/icons/view/sidebar-right-hide.svg
    ${FLUVEL_RESOURCES_DIR}/icons/view/sidebar-right-show.svg

    ${FLUVEL_RESOURCES_DIR}/icons/status/check-symbolic.svg

    ${FLUVEL_RESOURCES_DIR}/licenses/Licence_CeCILL_V2.1-en.txt
    ${FLUVEL_RESOURCES_DIR}/licenses/Licence_CeCILL_V2.1-fr.txt
)

foreach(res ${APP_RESOURCES})
    file(RELATIVE_PATH rel_path ${FLUVEL_RESOURCES_DIR} ${res})
    set_source_files_properties(
        ${res}
        PROPERTIES QT_RESOURCE_ALIAS ${rel_path}
    )
endforeach()

qt_add_resources(${FLUVEL_TARGET_NAME} app_resources
    PREFIX "/"
    FILES ${APP_RESOURCES}
)

target_include_directories(${FLUVEL_TARGET_NAME} PRIVATE
    ${CORE_DIR}
    ${UI_COMP_DIR}
    ${UI_WIN_DIR}
    ${INTEROP_DIR}
)

if(OpenCV_FOUND)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_USE_OPENCV)
endif()

if(Qt6_FOUND)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_USE_QT)
endif()


if(ENABLE_ASAN)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_ASAN)
else()
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:FLUVEL_DEBUG>
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${FLUVEL_TARGET_NAME} PRIVATE
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
    target_compile_options(${FLUVEL_TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
        $<$<CONFIG:MinSizeRel>:/O1>
    )
endif()

if(FLUVEL_UI_DESKTOP)
    target_compile_definitions(${FLUVEL_TARGET_NAME} PRIVATE FLUVEL_UI_DESKTOP)
endif()

set(APP_QT_LIBS
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Widgets
    Qt6::Svg
)

if(FLUVEL_USE_QML)
    list(APPEND APP_QT_LIBS
        Qt6::Qml
        Qt6::Quick
    )
endif()

target_link_libraries(${FLUVEL_TARGET_NAME} PRIVATE
    ${APP_QT_LIBS}
    fluvel::image_processing
)

include(${FLUVEL_ROOT_DIR}/cmake/fluvel_build_info.cmake)
