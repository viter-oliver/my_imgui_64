include_directories(${cuda_head_dir} ${cudnn_head_dir} ${torch_head_dir} ${torch_api_head_dir} ${torchvision_head_dir} ${opencv_head_dir})
link_directories(${cuda_lib_dir} ${cudnn_lib_dir} ${torch_lib_dir}
${torchvision_lib_dir} ${opencv_lib_dir})
file(GLOB model_src iou_cal.cpp iou_cal.h)
add_library(hud_lai SHARED ${model_src})
set_target_properties(hud_lai PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(hud_lai cuda cudnn torch opencv_world470d torchvision)
target_compile_definitions(hud_lai PRIVATE net_path="${CMAKE_CURRENT_SOURCE_DIR}/net")