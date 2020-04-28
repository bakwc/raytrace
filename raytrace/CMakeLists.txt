cmake_minimum_required(VERSION 3.5)


project(raytrace)

set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-O2 -g")

include_directories( ./include )
include_directories(/usr/local/include ${PROJECT_SOURCE_DIR}/include)
link_directories(/usr/local/lib)

add_executable(raytrace main.cpp opencl_raytracer.cpp metal_raytracer.cpp mtlpp.mm utils.cpp glad.c)

target_link_libraries(raytrace glfw3)
target_link_libraries(raytrace "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenCL -framework Metal")
