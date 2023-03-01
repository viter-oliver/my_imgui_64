include_directories(${PROJECT_SOURCE_DIR}/Font23D-master

)
file(GLOB ftgl ftgl/*.cpp)
set(shapes poly2tri/common/shapes.cc)
file(GLOB sweep poly2tri/sweep/*.cc)
add_executable(font23d ${ftgl} ${shapes} ${sweep} test.cpp)
target_link_libraries(font23d freetyped)