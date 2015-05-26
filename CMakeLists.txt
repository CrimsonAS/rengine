cmake_minimum_required(VERSION 2.6)
project(rengine)

find_package(Qt5Gui)
find_package(OpenGL)

include_directories(include 3rdparty)

enable_testing()

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

file(MAKE_DIRECTORY lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

file(MAKE_DIRECTORY bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

#
# librengine.a
#
add_library(rengine STATIC
    src/qt/qtbackend.cpp
    src/scenegraph/openglrenderer.cpp
)
target_link_libraries(rengine Qt5::Gui)


#
# examples/hello
#
add_executable(ex_hello examples/hello/main.cpp)
add_dependencies(ex_hello rengine)
target_link_libraries(ex_hello rengine)
target_link_libraries(ex_hello ${OPENGL_LIBRARIES})

#
# tst_node
#
add_executable(tst_node tests/tst_node.cpp)
add_test(tst_node tst_node)
add_dependencies(tst_node rengine)

#
# tst_mathtypes
#
add_executable(tst_mathtypes tests/tst_mathtypes.cpp)
add_test(tst_mathtypes tst_mathtypes)
add_dependencies(tst_mathtypes rengine)

#
# tst_keyframes
#
add_executable(tst_keyframes tests/tst_keyframes.cpp)
add_test(tst_keyframes tst_keyframes)
add_dependencies(tst_keyframes rengine)
