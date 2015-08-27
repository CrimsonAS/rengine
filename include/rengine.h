/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#define RENGINE_NAMESPACE_PREFIX rengine::
#define RENGINE_BEGIN_NAMESPACE namespace rengine {
#define RENGINE_END_NAMESPACE }
#define RENGINE_USE_NAMESPACE using namespace rengine;

// Forward declarations
RENGINE_BEGIN_NAMESPACE

// 'common' subdir
struct vec2;
struct vec3;
struct vec4;
struct mat4;

// 'windowsystem' subdir
class Surface;
class SurfaceInterface;

// 'scenegraph' subdir
class Node;
class OpacityNode;
class TextureNode;
class TransformNode;
class ColorFilterNode;
class Renderer;
class Texture;
class OpenGLRenderer;
class OpenGLTextureTexture;
class OpenGLShaderProgram;

// 'animation' subdir
template <typename T> class KeyFrameValuesBase;
template <typename V, typename T, typename AF> class KeyFrameValues;
template <typename T> class KeyFrames;

// toplevel include dir
class Backend;

// 'object' subdir

RENGINE_END_NAMESPACE

#include "common/mathtypes.h"
#include "common/allocationpool.h"
#include "common/colormatrix.h"

#include "windowsystem/surface.h"

#include "scenegraph/opengl.h"
#include "scenegraph/node.h"
#include "scenegraph/texture.h"
#include "scenegraph/renderer.h"
#include "scenegraph/openglshaderprogram.h"
#include "scenegraph/opengltexture.h"
#include "scenegraph/openglrenderer.h"

#include "animationsystem/animation.h"
#include "animationsystem/animationappliers.h"

#include "backend.h"

#include "util/standardsurfaceinterface.h"
#include "util/maindefine.h"

#include "object/property.h"
#include "object/signal.h"

#if defined RENGINE_BACKEND_QT
#include "backend/qt/qtbackend.h"
#elif defined RENGINE_BACKEND_SDL
#include "backend/sdl/sdlbackend.h"
#else
#error "Please define which backend you want... RENGINE_BACKEND_QT or RENGINE_BACKEND_SDL.. Thanks.."
#endif


