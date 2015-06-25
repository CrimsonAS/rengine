#pragma once

RENGINE_BEGIN_NAMESPACE

class StandardSurfaceInterface : public SurfaceInterface
{
public:
    StandardSurfaceInterface()
        : m_renderer(0)
    {
    }

    ~StandardSurfaceInterface()
    {
        delete m_renderer->sceneRoot();
        delete m_renderer;
    }

    virtual Node *update(Node *oldRoot) = 0;

    virtual void beforeRender() { }
    virtual void afterRender() { }

    void onRender() {
        surface()->makeCurrent();

        // Initialize the renderer if this is the first time around
        if (!m_renderer) {
            m_renderer = Backend::get()->createRenderer(surface());
            m_animationManager.start();
        }

        // Create the scene graph; update if it already exists..
        m_renderer->setSceneRoot(update(m_renderer->sceneRoot()));

        if (!m_renderer->sceneRoot())
            return;

        // Advance the animations just before rendering..
        m_animationManager.tick();


        // And then render the stuff
        beforeRender();
        m_renderer->render();
        afterRender();

        surface()->swapBuffers();
        m_renderer->frameSwapped();

        // Schedule a repaint again if there are animations running...

        // ### TODO: Optimize waiting for scheduled animations. Rather than
        // just keep on rendering, we should figure out how long we need to
        // wait and schedule an update at that time.
        if (m_animationManager.animationsRunning() || m_animationManager.animationsScheduled()) {
            surface()->requestRender();
        }
    }

    Renderer *renderer() const { return m_renderer; }

    AnimationManager *animationManager() { return &m_animationManager; }

private:
    Renderer *m_renderer;
    AnimationManager m_animationManager;
};

RENGINE_END_NAMESPACE
