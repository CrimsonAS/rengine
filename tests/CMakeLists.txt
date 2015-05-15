cmake_minimum_required(VERSION 2.6)
project(RendgineTests)
include_directories(../include)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

add_executable(tst_node tst_node.cpp)