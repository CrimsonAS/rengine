
this is 'rengine' - a rendering engine
--------------------------------------

Rengine compiles to a single static library and has no dependencies other than
OpenGL and the C++ standard library.

Optional dependences include:

 - Qt: for the Qt based backend (currently a hard dependency and only working backend)

 - Wayland: For the wayland based front end (currently not implented)


todo
----

Mostly a list of things to keep in mind for my own sake


overview of the dependencies between source directories
-------------------------------------------------------

include/rengine.h
 - main include file, used by all applications

include/common
 - Utility classes used throughout
 - no dependencies, can be used stand alone
 - all inline

include/scenegraph
 - The scene graph and default OpenGL renderer
 - depends on src/common

include/animation
 - The animation system
 - no dependences, can be used stand alone
 - all inline

include/windowsystem
 - The windowsystem/display/screen interfaces
 - depends on src/common

src/qt
 - Implementation of a Qt backend

src/sailfish
 - Implementation of a Sailfish system (todo :)

3rdparty
 - stb_image.h -> for easy image loading

tests
 - tst_node
 - tst_mathtypes
 - tst_keyframes

examples
 - hello -> sample to show it works