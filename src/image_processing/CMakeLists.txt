# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

# current image_processing directory
set(IP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(FILTERING_DIR       ${IP_DIR}/filtering)
set(SEGMENTATION_DIR    ${IP_DIR}/segmentation)
set(ANALYSIS_DIR        ${IP_DIR}/analysis)
set(CORE_DIR            ${IP_DIR}/core)

set(ip_src

    ${FILTERING_DIR}/image_pipeline.cpp
    ${FILTERING_DIR}/image_pipeline.hpp

    ${FILTERING_DIR}/processing_params.hpp

    ${FILTERING_DIR}/noise.cpp
    ${FILTERING_DIR}/noise.hpp

    ${FILTERING_DIR}/mean_filter.cpp
    ${FILTERING_DIR}/mean_filter.hpp

    ${FILTERING_DIR}/mean_filter_3x3.cpp
    ${FILTERING_DIR}/mean_filter_3x3.hpp

    ${FILTERING_DIR}/mean_filter_sliding.cpp
    ${FILTERING_DIR}/mean_filter_sliding.hpp

    ${FILTERING_DIR}/median_filter.cpp
    ${FILTERING_DIR}/median_filter.hpp

    ${FILTERING_DIR}/temporal_mean.cpp
    ${FILTERING_DIR}/temporal_mean.hpp

    ${FILTERING_DIR}/anisotropic_diffusion.cpp
    ${FILTERING_DIR}/anisotropic_diffusion.hpp

    ${FILTERING_DIR}/morpho.cpp
    ${FILTERING_DIR}/morpho.hpp

    ${FILTERING_DIR}/naive_morpho.hpp
    ${FILTERING_DIR}/van_herk_morpho.hpp

    ${FILTERING_DIR}/pixel_wise.cpp
    ${FILTERING_DIR}/pixel_wise.hpp


    ${SEGMENTATION_DIR}/contour_types.hpp
    ${SEGMENTATION_DIR}/active_contour_types.hpp
    ${SEGMENTATION_DIR}/speed_model_types.hpp

    ${SEGMENTATION_DIR}/boundary_builder.cpp
    ${SEGMENTATION_DIR}/boundary_builder.hpp

    ${SEGMENTATION_DIR}/contour_data.cpp
    ${SEGMENTATION_DIR}/contour_data.hpp

    ${SEGMENTATION_DIR}/active_contour.cpp
    ${SEGMENTATION_DIR}/active_contour.hpp

    ${SEGMENTATION_DIR}/speed_model.hpp

    ${SEGMENTATION_DIR}/majority_internal_speed.cpp
    ${SEGMENTATION_DIR}/majority_internal_speed.hpp

    ${SEGMENTATION_DIR}/region_gray_speed_model.cpp
    ${SEGMENTATION_DIR}/region_gray_speed_model.hpp

    ${SEGMENTATION_DIR}/region_color_speed_model.cpp
    ${SEGMENTATION_DIR}/region_color_speed_model.hpp

    ${SEGMENTATION_DIR}/contour_diagnostics.hpp

    ${ANALYSIS_DIR}/shape_conversion.cpp
    ${ANALYSIS_DIR}/shape_conversion.hpp

    ${ANALYSIS_DIR}/shape.cpp
    ${ANALYSIS_DIR}/shape.hpp

    ${ANALYSIS_DIR}/hausdorff_distance.cpp
    ${ANALYSIS_DIR}/hausdorff_distance.hpp


    ${CORE_DIR}/grid2d.hpp
    ${CORE_DIR}/image_format.hpp
    ${CORE_DIR}/image_view.hpp
    ${CORE_DIR}/image_owner.hpp
    ${CORE_DIR}/image_conversions.hpp
    ${CORE_DIR}/point.hpp
    ${CORE_DIR}/point_hash.hpp
    ${CORE_DIR}/point_containers.hpp
    ${CORE_DIR}/fluvel_math.hpp
    ${CORE_DIR}/color.hpp
    ${CORE_DIR}/neighborhood.hpp
    ${CORE_DIR}/elapsed_timer.hpp
)

add_library(${LIB_TARGET_NAME} STATIC ${ip_src})
add_library(fluvel::image_processing ALIAS ${LIB_TARGET_NAME})

set_target_properties(${LIB_TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${LIB_TARGET_NAME} PUBLIC
    ${CORE_DIR}
    ${ANALYSIS_DIR}
    ${SEGMENTATION_DIR}
    ${FILTERING_DIR}
)
