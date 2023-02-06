
include_directories(${PROJECT_SOURCE_DIR}/raw_image
)
file(GLOB cg_src *.cpp *.h)

add_executable(create_bmp ${cg_src} )