cmake_minimum_required(VERSION 2.6)
project(HelloRengine)
include_directories(../../include)

find_package(Qt5Gui)
find_package(OpenGL)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

add_executable(hello main.cpp)

target_link_libraries(hello rengine)
target_link_libraries(hello ${OPENGL_LIBRARIES})