#pragma once

namespace rengine {

class OpenGLRenderer : public Renderer
{
public:
    OpenGLRenderer();

    void setOpenGLContext(OpenGLContext *gl) { m_gl = gl; }

    void setSceneRoot(Node *root) { m_sceneRoot = root; }

    void setTargetSurface(Surface *surface) { m_surface = surface; }

    bool render();

private:
    Node *m_sceneRoot;
    Surface *m_surface;
    OpenGLContext *m_gl;
};

}