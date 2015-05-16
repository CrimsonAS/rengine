cmake_minimum_required(VERSION 2.6)
project(HelloRengine)
include_directories(../../include)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

add_executable(hello main.cpp)

target_link_libraries(hello rengine)
