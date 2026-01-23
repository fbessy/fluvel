
# current image_processing directory
set(IP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET_NAME ofeli_image_processing)

set(ALGO_DIR     ${IP_DIR}/algo)
set(CORE_DIR     ${IP_DIR}/core)
set(ADAPTERS_DIR ${IP_DIR}/adapters)


set(ip_src
    ${ALGO_DIR}/contour_data.cpp
    ${ALGO_DIR}/contour_data.hpp

    ${ALGO_DIR}/active_contour.cpp
    ${ALGO_DIR}/active_contour.hpp

    ${ALGO_DIR}/region_ac.cpp
    ${ALGO_DIR}/region_ac.hpp

    ${ALGO_DIR}/region_color_ac.cpp
    ${ALGO_DIR}/region_color_ac.hpp

    ${ALGO_DIR}/edge_ac.cpp
    ${ALGO_DIR}/edge_ac.hpp

    ${ALGO_DIR}/hausdorff_distance.cpp
    ${ALGO_DIR}/hausdorff_distance.hpp

    ${ALGO_DIR}/filters.cpp
    ${ALGO_DIR}/filters.hpp

    ${ALGO_DIR}/filters2.cpp
    ${ALGO_DIR}/filters2.hpp

    ${ALGO_DIR}/boundary_builder.cpp
    ${ALGO_DIR}/boundary_builder.hpp

    ${ALGO_DIR}/shape.cpp
    ${ALGO_DIR}/shape.hpp

    ${CORE_DIR}/grid2d.hpp
    ${CORE_DIR}/image_span.hpp
    ${CORE_DIR}/point.hpp
    ${CORE_DIR}/point_hash.hpp
    ${CORE_DIR}/ofeli_math.hpp
    ${CORE_DIR}/color.hpp

    ${ADAPTERS_DIR}/image_adapters.hpp
)

add_library(${TARGET_NAME} STATIC ${ip_src}
    core/neighborhood.hpp)
add_library(ofeli::image_processing ALIAS ${TARGET_NAME})


target_include_directories(${TARGET_NAME} PUBLIC
    ${ADAPTERS_DIR}
    ${CORE_DIR}
    ${ALGO_DIR}
)

# image_processing/
#   include/ofeli/image_processing/   <-- API publique
#   core/                             <-- interne
#   algo/                             <-- interne
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
