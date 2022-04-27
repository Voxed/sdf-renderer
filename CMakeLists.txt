cmake_minimum_required(VERSION 3.21)
project(Vx_Luminence)

set(CMAKE_CXX_STANDARD 14)

find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_subdirectory(third_party)

add_executable(Vx_Luminence main.cpp Mesh.cpp Mesh.h Primitive.cpp Primitive.h third_party/OBJ_Loader.h)
target_link_libraries(Vx_Luminence spdlog::spdlog glfw OpenGL::OpenGL GLEW::GLEW imgui_glfw)
