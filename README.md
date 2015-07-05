
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
   - at least for now..
   - Sort center of layers front to back and render them (like chrome does)
 - replacable backends to handle windowsystem/eventloop/input
   - optional backend code to hardware compositor
 - optional frontend to wayland to provide window surfaces as layers
   - application protocol to create and run animations
   - server-side pannables

Right now this project is pretty lame. It ain't stable, not very functional,
hard to use and very much work in progress, but I'm having fun :)

Now go look at something else...

Optional dependences include:

 - Qt: for the Qt based backend
 - SDL2: for the SDL 2 based backend, enable using 'cmake -DRENGINE_USE_SDL=on'
 - Wayland: For the wayland based front end (currently not implented)


todo
----

lots and lots...
 - OpenGL renderer
   - antialiased edges -> rely on MSAA for now, though this is slow on intel chips
   - filter nodes (maybe drop blur/dropshadow for now since they are expensive as hell)
      -> provide effects both as 'live' in the tree and 'static' as a means of producing a Layer instance.
   - caching of non-changing flattened subtrees to improve performance
 - add more properties to LayerNode
   - opacity
   - border and rounded edges? -> lets not for now...
 - API
   - Be consistent with use of 2D (mat4::translate2D) and 2d (rect2d)


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