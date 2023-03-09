file(GLOB code_src *.cpp *.h)
add_executable(hst_arhud ${code_src})
target_compile_definitions(hst_arhud PRIVATE IMGUI_DISABLE_DEMO_WINDOWS BUILD_WIN32)
target_link_libraries(hst_arhud HSG_LIB)