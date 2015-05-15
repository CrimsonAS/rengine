#pragma once

#include "renderer.h"

namespace Rengine {
namespace SceneGraph {

class OpenGLRenderer : public Renderer
{
public:
    OpenGLRenderer();

    void setSceneRoot(Node *root) { m_sceneRoot = root; }
    void render();

private:
    Node *m_sceneRoot;
};

}
}