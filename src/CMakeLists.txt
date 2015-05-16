cmake_minimum_required(VERSION 2.6)
project(Rengine)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

file(MAKE_DIRECTORY ../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)
add_library(rengine STATIC
    qt/qtsystem.cpp
)