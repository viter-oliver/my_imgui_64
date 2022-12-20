
include_directories(${PROJECT_SOURCE_DIR}/hsg_graphics_design

)
file(GLOB cg_src *.cpp *.h)
file(GLOB imgui_source ${PROJECT_SOURCE_DIR}/imgui/*.cpp  
${PROJECT_SOURCE_DIR}imgui/*.h)
list(REMOVE_ITEM imgui_source 
${PROJECT_SOURCE_DIR}/imgui/imgui_node_graph_test.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
)
add_executable(compositionGame ${cg_src} 
${imgui_source}
${PROJECT_SOURCE_DIR}/hsg_graphics_design/ft_base.cpp 
${PROJECT_SOURCE_DIR}/hsg_graphics_design/af_font_res_set.cpp 
${PROJECT_SOURCE_DIR}/hsg_graphics_design/af_timer.cpp
${PROJECT_SOURCE_DIR}/hsg_graphics_design/imgui_impl_glfw_gl3.cpp
)
target_link_libraries(compositionGame glfw gl3w freetyped opengl32)
target_compile_definitions(compositionGame PRIVATE 
IMGUI_DISABLE_DEMO_WINDOWS AFG_GAME WIN32_LEAN_AND_MEAN)