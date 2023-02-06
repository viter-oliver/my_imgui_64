include_directories(${PROJECT_SOURCE_DIR}/raw_image
)
file(GLOB cg_src *.cpp *.h)

add_executable(create_dic_txt ${cg_src} )