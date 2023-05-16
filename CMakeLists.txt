cmake_minimum_required(VERSION 3.0)

project(hello_opengl)

link_directories(${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

add_compile_definitions(GLEW_STATIC)

set(TUTORIAL "06_vcam_with_quaternion")

file(GLOB CPP_FILES src/${TUTORIAL}/*.cpp src/${TUTORIAL}/*.c)

add_executable(main ${CPP_FILES})

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/vendor)

file(GLOB math_source ${PROJECT_SOURCE_DIR}/vendor/maths_funcs.cpp ${PROJECT_SOURCE_DIR}/vendor/maths_funcs.h)
file(GLOB utils_source ${PROJECT_SOURCE_DIR}/vendor/gl_utils.cpp ${PROJECT_SOURCE_DIR}/vendor/gl_utils.h)
file(GLOB obj_source ${PROJECT_SOURCE_DIR}/vendor/obj_parser.cpp ${PROJECT_SOURCE_DIR}/vendor/obj_parser.h)
add_library(math OBJECT ${math_source})
add_library(utils OBJECT ${utils_source})
add_library(obj OBJECT ${obj_source})

target_link_libraries(main glfw3 opengl32 glew32s.lib math utils obj)