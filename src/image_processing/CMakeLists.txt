# 25 files

set(IP_SRC
    "boundary_builder.cpp" "boundary_builder.hpp"
    "contour_data.cpp" "contour_data.hpp"
    "active_contour.cpp" "active_contour.hpp"
    "region_ac.cpp" "region_ac.hpp"
    "region_color_ac.cpp" "region_color_ac.hpp"
    "edge_ac.cpp" "edge_ac.hpp"
    "hausdorff_distance.cpp" "hausdorff_distance.hpp"
    "filters.cpp" "filters.hpp"
    "shape.cpp" "shape.hpp"
    "matrix.txx" "matrix.hpp"
    "list_i.cpp" "list_i.hpp"
    "image_views.cpp" "image_views.hpp"
    "point.hpp"
)

qt_add_library(image_processing STATIC
    ${IP_SRC}
)

target_include_directories(image_processing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
