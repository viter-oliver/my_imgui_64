include_directories(${opencv_head_dir}
${PROJECT_SOURCE_DIR}/hsg_graphics_design
${PROJECT_SOURCE_DIR}/dbcppp/include
)
link_directories(${opencv_lib_dir}
${PROJECT_SOURCE_DIR}/dbcppp/lib/Debug
)
set(TargetName lane_fitting_analyser)
file(GLOB imgui_source ${PROJECT_SOURCE_DIR}/imgui/*.cpp  
${PROJECT_SOURCE_DIR}imgui/*.h)
list(REMOVE_ITEM imgui_source 
${PROJECT_SOURCE_DIR}/imgui/imgui_node_graph_test.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
)
set(dev_enumerator_src DeviceEnumerator.cpp DeviceEnumerator.h)
add_executable(${TargetName} 
${imgui_source} ${dev_enumerator_src}
${PROJECT_SOURCE_DIR}/hsg_graphics_design/imgui_impl_glfw_gl3.cpp
lane_fitting_analyser.cpp CanMessage.cpp CanMessage.h)
set_property(TARGET ${TargetName} PROPERTY CXX_STANDARD 17)
target_link_libraries(${TargetName}  glfw gl3w freetyped opengl32 
opencv_world470d libdbcppp libxmlmm libxml2d ControlCAN)
target_compile_definitions(${TargetName} PRIVATE IMGUI_DISABLE_DEMO_WINDOWS WIN32_LEAN_AND_MEAN )
