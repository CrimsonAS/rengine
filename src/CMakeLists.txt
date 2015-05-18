cmake_minimum_required(VERSION 2.6)
project(Rengine)

find_package(Qt5Gui)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

file(MAKE_DIRECTORY ../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
add_library(rengine STATIC
    qt/qtsystem.cpp
    scenegraph/openglrenderer.cpp
)

target_link_libraries(rengine Qt5::Gui)