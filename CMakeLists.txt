cmake_minimum_required(VERSION 3.0)

project(hello_opengl)

link_directories(${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

add_compile_definitions(GLEW_STATIC)

set(TUTORIAL "03_vertex_buffer_objects")

file(GLOB CPP_FILES src/${TUTORIAL}/*.cpp src/${TUTORIAL}/*.c)

add_executable(main ${CPP_FILES})

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/vendor)

target_link_libraries(main glfw3 opengl32 glew32s.lib)