#pragma once

RENGINE_BEGIN_NAMESPACE

class StandardSurfaceInterface : public SurfaceInterface
{
public:
    StandardSurfaceInterface()
        : m_renderer(0)
    {
    }

    virtual Node *update(Node *oldRoot) = 0;

    void onRender() {
        if (!m_renderer) {
            m_renderer = Backend::get()->createRenderer();
            m_renderer->setTargetSurface(surface());
            m_renderer->initialize();
        }

        m_renderer->setSceneRoot(update(m_renderer->sceneRoot()));
        m_renderer->render();
    }

    Renderer *renderer() const { return m_renderer; }

private:
    Renderer *m_renderer;
};

RENGINE_END_NAMESPACE
