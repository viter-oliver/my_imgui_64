
include_directories(${PROJECT_SOURCE_DIR}/hsg_graphics_design
${PROJECT_SOURCE_DIR}/../json-3.10.5/single_include
)
file(GLOB cg_src *.cpp *.h)
file(GLOB imgui_source ${PROJECT_SOURCE_DIR}/imgui/*.cpp  
${PROJECT_SOURCE_DIR}imgui/*.h)
list(REMOVE_ITEM imgui_source 
${PROJECT_SOURCE_DIR}/imgui/imgui_node_graph_test.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
)
add_executable(display_game ${cg_src} 
${imgui_source}
${PROJECT_SOURCE_DIR}/hsg_graphics_design/af_font_res_set.cpp 
${PROJECT_SOURCE_DIR}/hsg_graphics_design/af_timer.cpp
${PROJECT_SOURCE_DIR}/hsg_graphics_design/imgui_impl_glfw_gl3.cpp
${soil_src} ${soil_h}
)
target_link_libraries(display_game glfw gl3w freetyped opengl32)
target_compile_definitions(display_game PRIVATE 
IMGUI_DISABLE_DEMO_WINDOWS AFG_GAME WIN32_LEAN_AND_MEAN)