cmake_minimum_required(VERSION 3.10.0)
project(LearnOpenGL VERSION 0.1.0 LANGUAGES C CXX)

find_package(glfw3 REQUIRED)

add_executable(LearnOpenGL src/main.cpp src/glad.c)

target_include_directories(LearnOpenGL PRIVATE include)
target_link_libraries(LearnOpenGL glfw)

configure_file(src/vertex_s.glsl ./ COPYONLY)
configure_file(src/frag_s.glsl ./ COPYONLY)