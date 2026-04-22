
# current image_processing directory
set(IP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET_NAME fluvel_image_processing)

set(ALGO_DIR     ${IP_DIR}/algo)
set(CORE_DIR     ${IP_DIR}/core)

set(ip_src
    ${ALGO_DIR}/contour_data.cpp
    ${ALGO_DIR}/contour_data.hpp

    ${ALGO_DIR}/speed_model.hpp

    ${ALGO_DIR}/active_contour.cpp
    ${ALGO_DIR}/active_contour.hpp
    ${ALGO_DIR}/ac_types.hpp

    ${ALGO_DIR}/majority_internal_speed.cpp
    ${ALGO_DIR}/majority_internal_speed.hpp

    ${ALGO_DIR}/region_gray_speed_model.cpp
    ${ALGO_DIR}/region_gray_speed_model.hpp

    ${ALGO_DIR}/region_color_speed_model.cpp
    ${ALGO_DIR}/region_color_speed_model.hpp

    #${ALGO_DIR}/edge_ac.cpp
    #${ALGO_DIR}/edge_ac.hpp

    ${ALGO_DIR}/hausdorff_distance.cpp
    ${ALGO_DIR}/hausdorff_distance.hpp

    ${ALGO_DIR}/filters.cpp
    ${ALGO_DIR}/filters.hpp

    ${ALGO_DIR}/boundary_builder.cpp
    ${ALGO_DIR}/boundary_builder.hpp

    ${ALGO_DIR}/shape.cpp
    ${ALGO_DIR}/shape.hpp

    ${ALGO_DIR}/image_pipeline.cpp
    ${ALGO_DIR}/image_pipeline.hpp

    ${ALGO_DIR}/noise.cpp
    ${ALGO_DIR}/noise.hpp

    ${ALGO_DIR}/mean_filter.cpp
    ${ALGO_DIR}/mean_filter.hpp

    ${ALGO_DIR}/mean_filter_3x3.cpp
    ${ALGO_DIR}/mean_filter_3x3.hpp

    ${ALGO_DIR}/mean_filter_sliding.cpp
    ${ALGO_DIR}/mean_filter_sliding.hpp

    ${ALGO_DIR}/median_filter.cpp
    ${ALGO_DIR}/median_filter.hpp

    ${ALGO_DIR}/temporal_mean.cpp
    ${ALGO_DIR}/temporal_mean.hpp

    ${ALGO_DIR}/anisotropic_diffusion.cpp
    ${ALGO_DIR}/anisotropic_diffusion.hpp

    ${ALGO_DIR}/morpho.cpp
    ${ALGO_DIR}/morpho.hpp

    ${ALGO_DIR}/naive_morpho.hpp
    ${ALGO_DIR}/van_herk_morpho.hpp

    ${ALGO_DIR}/pixel_wise.cpp
    ${ALGO_DIR}/pixel_wise.hpp

    ${ALGO_DIR}/contour_diagnostics.hpp

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

add_library(${TARGET_NAME} STATIC ${ip_src})
add_library(fluvel::image_processing ALIAS ${TARGET_NAME})


target_include_directories(${TARGET_NAME} PUBLIC
    ${CORE_DIR}
    ${ALGO_DIR}
)

# image_processing/
#   include/fluvel/image_processing/   <-- API publique
#   core/                              <-- interne
#   algo/                              <-- interne
# Et côté CMake :

# cmake
# Copier le code
# target_include_directories(${TARGET_NAME}
#     PUBLIC
#         ${IP_DIR}/include
#     PRIVATE
#         ${CORE_DIR}
#         ${ALGO_DIR}
# )
