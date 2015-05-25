
THIS IS 'RENGINE' - A RENDERING ENGINE
--------------------------------------



Rengine compiles to a single static library and has no dependencies other than
OpenGL and the C++ standard library.

Optional dependences include:

 - Qt: for the Qt based backend
 - Wayland: For the wayland based front end


TODO
----

Mostly a list of things to keep in mind for my own sake


OVERVIEW OF THE DEPENDENCIES BETWEEN SOURCE DIRECTORIES
-------------------------------------------------------

include/rengine.h
    -> main include file, used by all applications

src/common
    -> Utility classes used throughout
    -> no dependencies, can be used stand alone

src/scenegraph
    -> The scene graph and default OpenGL renderer
    -> depends on src/common

src/animationsystem
    -> The animation system
    -> no dependences, can be used stand alone

src/animationtypes
    -> depends on src/common
    -> depends on src/scenegraph

src/windowsystem
    -> The windowsystem/display/screen interfaces
    -> depends on src/common

src/qt
    -> Implementation of a Qt backend

src/sailfish
    -> Implementation of a Sailfish system (to come...)

src
    -> Where the bits gets pulled together
    -> depends on src/common
    -> depends on src/scenegraph
    -> depends on src/windowsystem
    -> depends on src/animationsystem
    -> depends on src/animationtypes

3rdparty
    -> stb_image.h -> for easy image loading

tests
    -> tst_node
    -> tst_mathtypes
    -> tst_animationsystem
