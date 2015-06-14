cmake_minimum_required(VERSION 2.6)
project(rengine)

file(MAKE_DIRECTORY lib)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)



################################################################################
#
# Options
#
option(RENGINE_LOG_INFO "LogInfo" ON)
if (RENGINE_LOG_INFO)
    add_definitions(-DRENGINE_LOG_INFO)
endif()

option(RENGINE_OPENGL_FTB "OpenGL renders front-to-back" ON)
if (RENGINE_OPENGL_FTB)
    add_definitions(-DRENGINE_OPENGL_FTB)
endif()



################################################################################
#
# Macros
#
macro(add_rengine_example name)
    set(EXAMPLE_NAME "ex_${name}")
    add_executable(${EXAMPLE_NAME} "examples/${name}.cpp")
    add_dependencies(${EXAMPLE_NAME} rengine)
    target_link_libraries(${EXAMPLE_NAME} rengine ${LIBS_OPENGL})
endmacro(add_rengine_example)

macro(add_rengine_test name)
    set(TEST_NAME "tst_${name}")
    add_executable(${TEST_NAME} "tests/tst_${name}.cpp")
    add_test(${TEST_NAME} ${TEST_NAME})
    add_dependencies(${TEST_NAME} rengine)
    target_link_libraries(${TEST_NAME} rengine ${LIBS_OPENGL})
endmacro(add_rengine_test)



################################################################################
#
# Resolving libraries
#
find_package(Qt5Gui)
find_package(OpenGL)

if (OPENGL_FOUND)
    message("OpenGL (Desktop) was detected")
    set(LIBS_OPENGL ${OPENGL_LIBRARIES})
    add_definitions(-DRENGINE_OPENGL_DESKTOP)
else()
    message(FATAL_ERROR, "OpenGL not detected")
endif()



################################################################################
#
# Misc..
#
include_directories(include 3rdparty)
enable_testing()



################################################################################
#
# librengine.a
#
add_library(rengine STATIC
    src/qt/qtbackend.cpp
    src/scenegraph/openglrenderer.cpp
)
target_link_libraries(rengine Qt5::Gui)



################################################################################
#
# Tests and Examples
#
add_rengine_example(rectangles)
add_rengine_example(layeredopacity)

add_rengine_test(node)
add_rengine_test(mathtypes)
add_rengine_test(keyframes)
add_rengine_test(render)
