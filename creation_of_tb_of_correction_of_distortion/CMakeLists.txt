include_directories(${opencv_head_dir}
${PROJECT_SOURCE_DIR}/hsg_graphics_design
${cuda_head_dir} ${cudnn_head_dir}
${PROJECT_SOURCE_DIR}/lane_fitting_analyser/
${PROJECT_SOURCE_DIR}/fast-lzma2
)
link_directories(${opencv_lib_dir}
${cuda_lib_dir} ${cudnn_lib_dir}
${PROJECT_SOURCE_DIR}/dbcppp/lib/Debug
)
file(GLOB imgui_source ${PROJECT_SOURCE_DIR}/imgui/*.cpp  
${PROJECT_SOURCE_DIR}imgui/*.h)
list(REMOVE_ITEM imgui_source 
${PROJECT_SOURCE_DIR}/imgui/imgui_node_graph_test.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
)
list(APPEND imgui_source ${PROJECT_SOURCE_DIR}/hsg_graphics_design/imgui_impl_glfw_gl3.cpp
${PROJECT_SOURCE_DIR}/lane_fitting_analyser/DeviceEnumerator.cpp
${PROJECT_SOURCE_DIR}/lane_fitting_analyser/DeviceEnumerator.h
)
set(edit_src creator_dt.cpp create_txt_dic.cpp create_txt_dic.h 
video_dev_mg.cpp video_dev_mg.h can_dev_mg.cpp can_dev_mg.h 
can_dev_hmi.cpp can_dev_hmi.h
)
set(TargetName creator_dt)
add_executable(${TargetName} ${imgui_source} ${edit_src})
set_target_properties(${TargetName} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${TargetName} glfw gl3w freetyped opengl32 cuda cudnn opencv_world470d ControlCAN lzma)
target_compile_definitions(${TargetName} PRIVATE IMGUI_DISABLE_DEMO_WINDOWS WIN32_LEAN_AND_MEAN )