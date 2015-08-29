
a rendering engine
------------------

rengine is a header-only rendering library and has no dependencies other than
OpenGL and the C++ standard library (C++11).

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

Right now this project is not stable, not very functional,
hard to use and very much work in progress, but I'm having fun :)

Now go look at something else...

Optional dependences include:

 - Qt: for the Qt based backend, the default
 - SDL2: for the SDL 2 based backend, enable using 'cmake -DRENGINE_USE_SDL=on'


todo
----

lots and lots...
 - OpenGL renderer
   - antialiased edges -> rely on MSAA for now, though this is slow on intel chips
   - provide effects both as 'live' in the tree and 'static' as a means of producing a Texture instance.
   - caching of non-changing flattened subtrees to improve performance, especially on blurred subtrees
   - custom render node
 - add more properties to TextureNode
   - opacity
   - border and rounded edges? -> lets not for now...
 - API
   - Be consistent with use of 2D (mat4::translate2D) and 2d (rect2d)
   - Change: T *Node::from<T>(Node *)  ->  static T *T::from(Node *);
   - Combined matrices are hard in the animation system, for instance, setting
     up a rotation around a point x,y requires two nodes right now. One for the
     offset and one to run the animation on.
 - input, both keyboard, touch and mouse
   - shared input event for touch/pointer
   - what does the event receiver look like?
   - How does it fit with the tree


overview of the dependencies between source directories
-------------------------------------------------------

include/rengine.h
 - main include file, used by all applications

include/common
 - Utility classes used throughout
 - no dependencies, can be used stand alone

include/scenegraph
 - The scene graph and default OpenGL renderer
 - depends on src/common

include/animation
 - The animation system
 - no dependences, can be used stand alone

include/windowsystem
 - The windowsystem/display/screen interfaces
 - depends on src/common

include/backend/
 - Implementation of backends
    -> qt/qtbackend.h: for qt one
    -> sdl/sdlbackend.h: for the sdl one

src/sailfish
 - Implementation of a Sailfish system (todo :)

3rdparty
 - stb headers for reading and writing

tests
 - tst_keyframes: tests for the animation system
 - tst_mathtypes: unit tests for the math classes
 - tst_node: unit tests for node classes
 - tst_property: unit tests for the property concept
 - tst_render: unit tests for rendering
 - tst_signal: unit tests for the signal concept

examples - The examples are simple snippets meant to illustrate how a concept works
 - ex_benchmark_rectangles: benchmark on creating/destroying 1000 rects per frame, including rendering
 - ex_blur: shows the blurring
 - ex_filters: shows how color filtering works
 - ex_layeredopacity: shows layered opacity
 - ex_rectangles: shows to render with 3D projection
 - ex_shadow: shows drop shadow
