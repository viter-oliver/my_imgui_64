include_directories(${PROJECT_SOURCE_DIR}/hsg_graphics_design
${PROJECT_SOURCE_DIR}/af_include
)
add_executable(change_lane_prm 
change_lane_prm.cpp 
${PROJECT_SOURCE_DIR}/hsg_graphics_design/primitive_format.cpp
)