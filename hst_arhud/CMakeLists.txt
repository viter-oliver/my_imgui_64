set(debug_src debug_var_set.cpp debug_var_set.h)
set(app_src afg_engine.cpp afg_engine.h)
set(msg_src msg_host.cpp msg_host.h)
set(navi_src navi_turn.cpp navi_turn.h)
set(tar_approaching_src tar_approaching.cpp tar_approaching.h)
if(NOT DEFINED TAGRGET_NAME)
set(TAGRGET_NAME hst_rehearsal)
endif()
add_executable(${TAGRGET_NAME} 
${debug_src} ${app_src} ${msg_src} ${tar_approaching_src} 
${navi_src} register_msg_host3.cpp)
target_compile_definitions(${TAGRGET_NAME} PRIVATE IMGUI_DISABLE_DEMO_WINDOWS BUILD_WIN32)
target_link_libraries(${TAGRGET_NAME} HSG_LIB)