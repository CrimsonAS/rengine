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
        if (m_renderer->sceneRoot())
            m_renderer->sceneRoot()->destroy();
        delete m_renderer;
    }

    virtual Node *update(Node *oldRoot) = 0;

    virtual void beforeRender() { }
    virtual void afterRender() { }

    void onRender() override {
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

    void dispatchEvent(Event *e) override {

    }

    Renderer *renderer() const { return m_renderer; }

    AnimationManager *animationManager() { return &m_animationManager; }

private:
    Renderer *m_renderer;
    AnimationManager m_animationManager;
};

RENGINE_END_NAMESPACE
