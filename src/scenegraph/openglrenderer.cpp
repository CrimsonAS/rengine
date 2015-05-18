#include "../system.h"

#include <stdio.h>

using namespace rengine;

OpenGLRenderer::OpenGLRenderer()
    : m_sceneRoot(0)
{

}

bool OpenGLRenderer::render()
{
    printf("%s\n", __PRETTY_FUNCTION__);
    return true;
}