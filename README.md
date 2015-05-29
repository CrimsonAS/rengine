
a rendering engine
------------------

rengine compiles to a single static library and has no dependencies other than
OpenGL and the C++ standard library (C++11).

Right now, it is all pretty lame, but I'm having fun..

The idea is provide a simple scene graph and animation api that is capable of
support HTML/CSS style layer composition. That means:
 - CSS keyframe animations
 - CSS filters (grayscale, blur, etc)
 - (not the css parsing and logic itself, just the rendering and animation
   primitives for it)
 - only textures and solids
 - "dumb" 3D support
 - replacable backends to handle windowsystem/eventloop/input
 - optional backend code to hardware compositor
 - optional frontend to wayland to provide window surfaces as layers
   - application protocol to create and run animations
   - server-side pannables

Right now this project is pretty lame. It ain't stable, not very functional,
hard to use and very much work in progress, but I'm having fun :)

Now go look at something else...

Optional dependences include:

 - Qt: for the Qt based backend (currently a hard dependency and only working backend)
 - Wayland: For the wayland based front end (currently not implented)


todo
----

lots and lots...
 - OpenGL renderer
   - antialiased edges
   - layer flattening under opacity and filter nodes
   - filter nodes (maybe drop blur/dropshadow for now since they are expensive as hell)
   - caching of non-changing flattened subtrees to improve performance
 - add more properties to LayerNode
   - x, y
   - opacity
   - border and rounded edges?

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