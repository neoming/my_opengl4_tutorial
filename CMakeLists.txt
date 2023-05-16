cmake_minimum_required(VERSION 3.0)

project(hello_opengl)

link_directories(${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

add_compile_definitions(GLEW_STATIC)

## stutorial source path
set(TUTORIAL0 "00_hello_trangle")
set(TUTORIAL1 "01_extended_init")
set(TUTORIAL2 "02_shaders")
set(TUTORIAL3 "03_vertex_buffer_objects")
set(TUTORIAL4 "04_mat_and_vec")
set(TUTORIAL5 "05_virtual_camera")
set(TUTORIAL6 "06_vcam_with_quaternion")
set(TUTORIAL7 "07_ray_picking")
set(TUTORIAL8 "08_phong")

set(TUTORIAL ${TUTORIAL8})

file(GLOB CPP_FILES src/${TUTORIAL}/*.cpp src/${TUTORIAL}/*.c src/${TUTORIAL}/*.h)

add_executable(main ${CPP_FILES})

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/vendor)

target_link_libraries(main glfw3 opengl32 glew32s.lib)
