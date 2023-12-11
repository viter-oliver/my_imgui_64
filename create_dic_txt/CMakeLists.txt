include_directories(${PROJECT_SOURCE_DIR}/raw_image
${opencv_head_dir}
)
link_directories(${opencv_lib_dir}
)
add_executable(create_dic_txt create_dic_txt.cpp 
create_dic.cpp create_dic.h)
add_executable(create_txt_dic create_txt_dic.cpp)
target_link_libraries(create_txt_dic opencv_world470d)
add_executable(updateBrightnessConstrast updateBrightnessContrast.cpp)
target_link_libraries(updateBrightnessConstrast opencv_world470d)
add_executable(findContours findContours.cpp)
target_link_libraries(findContours  opencv_world470d)
add_executable(findContours1 findContours1.cpp)
target_link_libraries(findContours1  opencv_world470d)