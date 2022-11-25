include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../from_nzh/lcm-1.4.0
${CMAKE_CURRENT_SOURCE_DIR}/../../from_nzh/bd_lcm140/lcm
)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external_lib)
file(GLOB code_src *.cpp *.h)
add_executable(HSG_APP ${code_src})
target_compile_definitions(HSG_APP PRIVATE IMGUI_DISABLE_DEMO_WINDOWS BUILD_WIN32)
target_link_libraries(HSG_APP HSG_LIB lcm)