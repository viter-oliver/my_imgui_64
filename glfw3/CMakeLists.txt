file(GLOB glfw_h *.h)
file(GLOB glfw_s *.c)
add_library(glfw ${glfw_s} ${glfw_h})