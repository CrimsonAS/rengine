cmake_minimum_required(VERSION 2.6)
project(rengine)

file(MAKE_DIRECTORY lib)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


################################################################################
#
# Options
#
option(RENGINE_LOG_INFO "LogInfo" ON)
if (RENGINE_LOG_INFO)
    message("LogInfo: enabled")
    add_definitions(-DRENGINE_LOG_INFO)
endif()

option(RENGINE_LOG_DEBUG "LogDebug" OFF)
if (RENGINE_LOG_DEBUG)
    message("LogDebug: enabled")
    add_definitions(-DRENGINE_LOG_DEBUG)
endif()

option(RENGINE_LOG_WARNING "LogWarning" ON)
if (RENGINE_LOG_WARNING)
    message("LogWarning: enabled")
    add_definitions(-DRENGINE_LOG_WARNING)
endif()

option(RENGINE_LOG_ERROR "LogError" ON)
if (RENGINE_LOG_ERROR)
    message("LogError: enabled")
    add_definitions(-DRENGINE_LOG_ERROR)
endif()

option(RENGINE_USE_SDL "SDL Backend" OFF)



################################################################################
#
# Macros
#
macro(add_rengine_example name)
    set(EXAMPLE_NAME "ex_${name}")
    add_executable(${EXAMPLE_NAME} "examples/${name}.cpp")
    target_link_libraries(${EXAMPLE_NAME} ${RENGINE_LIBS})
endmacro(add_rengine_example)

macro(add_rengine_test name)
    set(TEST_NAME "tst_${name}")
    add_executable(${TEST_NAME} "tests/tst_${name}.cpp")
    add_test(${TEST_NAME} ${TEST_NAME})
    target_link_libraries(${TEST_NAME} ${RENGINE_LIBS})
endmacro(add_rengine_test)



################################################################################
#
# Resolving backend stuff
#
if(RENGINE_USE_SFHWC)
    message("-- SFHWC backend")
    add_definitions(-DRENGINE_BACKEND_SFHWC)
    include_directories(# android headers
                        /usr/lib/droid-devel/droid-headers
                        # libhybris headers
                        /usr/include/hybris/hwcomposerwindow
                        /usr/include/hybris/eglplatformcommon)
    set(RENGINE_LIBS ${RENGINE_LIBS}
        -lhardware
        -lhybris-common
        -lhybris-eglplatformcommon
        -lhybris-hwcomposerwindow
        -lsync
        -lEGL
        -lpthread
        -lmtdev)
else() # (RENGINE_USE_SDL)
    message("-- SDL backend")
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})
    if (LINUX)
        set(RENGINE_LIBS ${RENGINE_LIBS} -lpthread)
    endif()
    set(RENGINE_LIBS ${RENGINE_LIBS} -L/usr/local/lib ${SDL2_LIBRARIES})
    add_definitions(-DRENGINE_BACKEND_SDL)

endif()


find_package(OpenGL)
if (OPENGL_FOUND)
    set(RENGINE_LIBS ${RENGINE_LIBS} ${OPENGL_LIBRARIES})
    add_definitions(-DRENGINE_OPENGL_DESKTOP)
else()
    message(WARNING, "OpenGL was not found, assuming OpenGL ES 2.0 in default locations...")
    set(RENGINE_LIBS ${RENGINE_LIBS} -lGLESv2)
endif()






################################################################################
#
# Misc..
#
add_executable(renginec "src/renginec/main.cpp")
include_directories(include 3rdparty)
enable_testing()



################################################################################
#
# Tests and Examples
#
# add_rengine_example(rectangles)
# add_rengine_example(layeredopacity)
add_rengine_example(benchmark_rectangles)
# add_rengine_example(filters)
# add_rengine_example(blur)
# add_rengine_example(shadow)
# add_rengine_example(benchmark_blend)
# add_rengine_example(touch)
# add_rengine_example(text)

add_executable(demo
                    "examples/button.cpp"
                    "examples/imageutils.cpp"
                    "examples/main.cpp"
                    "examples/rootwindow.cpp"
                    )
target_link_libraries(demo ${RENGINE_LIBS})


add_rengine_test(node)
add_rengine_test(mathtypes)
#add_rengine_test(keyframes)
add_rengine_test(render)
add_rengine_test(property)
add_rengine_test(signal)
add_rengine_test(layout)
add_rengine_test(workqueue)
add_rengine_test(units)
