#pragma once

namespace rengine {

class OpenGLRenderer : public Renderer
{
public:
    OpenGLRenderer();

    void setSceneRoot(Node *root) { m_sceneRoot = root; }
    bool render();

private:
    Node *m_sceneRoot;
};

}