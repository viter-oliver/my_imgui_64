cmake_minimum_required(VERSION 2.8)
project(mgui C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include_directories(./
af_include
lua
fab
json
glfw3/include
gl3w
glm
imgui
soil
misc/freetype
simple_http
../boost_1_79_0
../json-3.10.5/single_include
../msgpack-c
../assimp-5.2.4/include
../freetype-2.6.5/include
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/running)
add_compile_definitions(_GLFW_WIN32 
_CRT_SECURE_NO_WARNINGS
WIN32_LEAN_AND_MEAN
)
set(boost_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../boost_1_79_0/stage/lib)
set(assimp_dg_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../assimp-5.2.4/build/lib/Debug)
set(assimp_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../assimp-5.2.4/build/lib/Release)
set(freetype_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../freetype-2.6.5/bd/Release)
set(freetype_dg_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../freetype-2.6.5/bd/Debug)
file(GLOB lua_h lua/*.h)
aux_source_directory(lua lua_src)
list(REMOVE_ITEM lua_src lua/lua.c lua/luac.c)
file(GLOB hsg_h af_include/*.h)
aux_source_directory(hsg_graphics_design hsg_src)
aux_source_directory(simple_http_src simple_http)
list(APPEND hsg_src hsg_graphics_design/hsg.rc)
list(REMOVE_ITEM hsg_src hsg_graphics_design/afb_load.cpp 
hsg_graphics_design/application.cpp
hsg_graphics_design/main_app.cpp
hsg_graphics_design/python_interpreter.cpp
hsg_graphics_design/texture_shader.cpp
hsg_graphics_design/screen_image_distortion.cpp
)
file(GLOB imgui_h imgui/*.h)
aux_source_directory(imgui imgui_src)
list(REMOVE_ITEM imgui_src imgui/imgui_node_graph_test.cpp
imgui/imgui_demo.cpp)
file(GLOB json_h json/*.h json/*.inl)
file(GLOB json_src json/*.cpp)
file(GLOB soil_h soil/*.h)
file(GLOB soil_src soil/*.c)
file(GLOB gl3w_src gl3w/GL/*.c gl3w/GL/*.h)
add_subdirectory(glfw3)
link_directories(${boost_lib_dir} ${assimp_dg_lib_dir}
 ${freetype_dg_lib_dir})
source_group("lua" FILES ${lua_src} ${lua_h})
source_group("gl3w" FILES ${gl3w_src})
source_group("imgui" FILES ${imgui_src} ${imgui_h})
source_group("hsg" FILES ${hsg_src} ${hsg_h})
source_group("json" FILES ${json_src} ${json_h})
source_group("soil" FILES ${soil_src} ${soil_h})
add_library(gl3w ${gl3w_src})
add_executable(HSG ${lua_src} ${lua_h} ${hsg_src} ${hsg_h}
 ${imgui_src} ${json_src} ${json_h} ${soil_src} ${soil_h}
 ${simple_http}
 )
 target_link_libraries( HSG glfw gl3w freetyped
 assimp-vc142-mtd shlwapi opengl32 HtmlHelp)
 file(GLOB ctl_src hsg_graphics_design/ft_*.cpp af_include/ft_*.h)
 file(GLOB rnd_src hsg_graphics_design/af_*.cpp hsg_graphics_design/af*.h)
 list(APPEND rnd_src hsg_graphics_design/lua_interpreter.cpp
 hsg_graphics_design/shader_uf.cpp
 hsg_graphics_design/application.cpp
 hsg_graphics_design/screen_image_distortion.cpp
 hsg_graphics_design/afb_load.cpp
 hsg_graphics_design/common_functions.cpp
 hsg_graphics_design/easing.cpp
 hsg_graphics_design/res_output.cpp
 hsg_graphics_design/rescontainer_manager.cpp
 hsg_graphics_design/paricles_system.cpp
 hsg_graphics_design/imgui_impl_glfw_gl3.cpp
 )

 source_group("control" FILES ${ctl_src})
 source_group("render_utilities" FILES ${rnd_src})
 add_library(HSG_LIB SHARED ${lua_src} ${lua_h} ${ctl_src} ${rnd_src}
 ${gl3w_src} ${imgui_src} ${json_src} ${json_h} ${soil_src} ${soil_h})
 target_link_libraries(HSG_LIB gl3w glfw freetyped opengl32)
 target_compile_definitions(HSG_LIB PRIVATE 
 IMGUI_DISABLE_DEMO_WINDOWS BUILD_DLL)
 add_subdirectory(demo)
 add_subdirectory(compositionGame)
 add_subdirectory(create_bmp)
 add_subdirectory(create_dic_txt)
 add_subdirectory(create_primitive_format_table)
 add_subdirectory(change_lane_prm)







