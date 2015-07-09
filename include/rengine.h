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
#include "scenegraph/openglrenderer.h"
#include "scenegraph/opengltexture.h"

#include "animationsystem/animation.h"
#include "animationsystem/animationappliers.h"

#include "backend.h"

#include "util/standardsurfaceinterface.h"
#include "util/maindefine.h"



