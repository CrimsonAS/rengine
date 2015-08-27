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

option(RENGINE_USE_SDL "SDL Backend" OFF)


################################################################################
#
# Macros
#
macro(add_rengine_example name)
    set(EXAMPLE_NAME "ex_${name}")
    add_executable(${EXAMPLE_NAME} "examples/${name}.cpp")
    add_dependencies(${EXAMPLE_NAME} rengine)
    target_link_libraries(${EXAMPLE_NAME} rengine ${RENGINE_LIBS})
endmacro(add_rengine_example)

macro(add_rengine_test name)
    set(TEST_NAME "tst_${name}")
    add_executable(${TEST_NAME} "tests/tst_${name}.cpp")
    add_test(${TEST_NAME} ${TEST_NAME})
    add_dependencies(${TEST_NAME} rengine)
    target_link_libraries(${TEST_NAME} rengine ${RENGINE_LIBS})
endmacro(add_rengine_test)



################################################################################
#
# Resolving libraries
#
if (RENGINE_USE_SDL)
    message("Building with SDL2")
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})
    # ### TODO: Why is the -L needed here?
    set(RENGINE_LIBS ${RENGINE_LIBS} -L/usr/local/lib)
else()
    message("Building with Qt")
    find_package(Qt5Gui)
endif()

find_package(OpenGL)


if (OPENGL_FOUND)
    message("OpenGL (Desktop) was detected")
    set(RENGINE_LIBS ${RENGINE_LIBS} ${OPENGL_LIBRARIES})
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
if (RENGINE_USE_SDL)
    add_library(rengine STATIC
                src/sdl/sdlbackend.cpp
               )
    target_link_libraries(rengine ${SDL2_LIBRARIES})
else()
    # Qt based
    add_library(rengine STATIC
                src/qt/qtbackend.cpp
               )
    target_link_libraries(rengine Qt5::Gui)
endif()



################################################################################
#
# Tests and Examples
#
add_rengine_example(rectangles)
add_rengine_example(layeredopacity)
add_rengine_example(benchmark_rectangles)
add_rengine_example(filters)
add_rengine_example(blur)
add_rengine_example(shadow)

add_rengine_test(node)
add_rengine_test(mathtypes)
add_rengine_test(keyframes)
add_rengine_test(render)
add_rengine_test(property)
add_rengine_test(signal)
