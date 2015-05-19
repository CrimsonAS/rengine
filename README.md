
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
 - Rename Surface -> Layer
 - Rename SurfaceNode -> LayerNode
 - Rename Window -> Surface


OVERVIEW OF THE DEPENDENCIES BETWEEN SOURCE DIRECTORIES
-------------------------------------------------------

include/rengine.h
    -> main include file, used by all applications

src/common
    -> Utility classes used throughout
    -> no dependencies

src/scenegraph
    -> The scene graph and default OpenGL renderer
    -> depends on src/common

src/animation
    -> The animation system
    -> depends on src/common
    -> depends on src/scenegraph?

src/windowsystem
    -> The windowsystem/display/screen interfaces
    -> depends on src/common

src/qt
    -> Implementation of a Qt system

src/sailfish
    -> Implementation of a Sailfish system (to come...)

src
    -> Where the bits gets pulled together
    -> depends on src/common
    -> depends on src/scenegraph
    -> depends on src/windowsystem

3rdparty
    -> Contains bundled other libraries, used by apps and examples
