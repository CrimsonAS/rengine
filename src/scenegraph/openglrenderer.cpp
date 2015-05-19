#include "../system.h"

#include <stdio.h>

using namespace rengine;

OpenGLRenderer::OpenGLRenderer()
    : m_sceneRoot(0)
{

}

bool OpenGLRenderer::render()
{
    m_gl->makeCurrent(m_surface);

    static int i = 0;
    if (++i > 1)
        i = 0;
    glClearColor(i, 0, 1 - i, 1);

    glClear(GL_COLOR_BUFFER_BIT);

    m_gl->swapBuffers(m_surface);

    return true;
}