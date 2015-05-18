#pragma once

#if defined(__APPLE__)
// OSX
# include <OpenGL/gl.h>

#else
// Other
#include <EGL/egl.h>
#include <GLESv2/gl2.h>
#endif