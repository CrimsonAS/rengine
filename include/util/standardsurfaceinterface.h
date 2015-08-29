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
    {
    }

    ~StandardSurfaceInterface()
    {
        if (m_renderer->sceneRoot())
            m_renderer->sceneRoot()->destroy();
        delete m_renderer;
    }

    virtual Node *update(Node *oldRoot) = 0;

    virtual void onBeforeRender() { }
    virtual void onAfterRender() { }

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
        onBeforeRender();
        m_renderer->render();
        onAfterRender();

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

    void unregisterPointerTarget(Node *n) {
        assert(n->isPointerTarget());
        n->setPointerTarget(false);

    }


    /*!

        Register \a node as a receiver of pointer events. When a pointer event
        occurs, it will be sent through the scene and transformed according to

     */
    void registerPointerTarget(Node *n) {
        assert(!n->isPointerTarget());
        n->setPointerTarget(true);
    }

    virtual bool onPointerEvent(Node *node, PointerEvent *event) {
        return true;
    }

    virtual void onEvent(Event *e) override;

    Renderer *renderer() const { return m_renderer; }
    AnimationManager *animationManager() { return &m_animationManager; }

protected:
    bool deliverPointerEventInScene(Node *n, PointerEvent *e);

    Renderer *m_renderer = nullptr;
    AnimationManager m_animationManager;
};

inline void StandardSurfaceInterface::onEvent(Event *e)
{
    switch (e->type()) {
    case Event::PointerDown:
    case Event::PointerUp:
    case Event::PointerMove:
        if (m_renderer && m_renderer->sceneRoot()) {
            PointerEvent *pe = PointerEvent::from(e);
            Node::dump(m_renderer->sceneRoot());
            deliverPointerEventInScene(m_renderer->sceneRoot(), pe);
        }
        break;
    default:
        std::cerr << __PRETTY_FUNCTION__ << ": unknown event type=" << e->type() << std::endl;
        break;

    }
}

inline bool StandardSurfaceInterface::deliverPointerEventInScene(Node *node, PointerEvent *e)
{
    assert(node);
    assert(e);

    std::cout << " - delivering: " << node << std::endl;

    Node::Type type = node->type();
    vec2 pos = e->position();

    // Transform the event if required..
    if (type == Node::TransformNodeType)
        e->setPosition(static_cast<TransformNode *>(node)->matrix() * pos);

    // Traverse children in backwards order (and bottom up), so we get
    // inverse-paint order delivery
    Node *child = node->lastChild();
    while (child) {
        if (deliverPointerEventInScene(child, e))
            return true;
        child = child->previousSibling();
        std::cout << " - now moving on to: " << child << std::endl;
    }
    if (node->isPointerTarget()) {
        const rect2d *area = 0;
        if (RectangleNode *rn = Node::from<RectangleNode>(node))
            area = &rn->geometry();
        else if (TextureNode *tn = Node::from<TextureNode>(node))
            area = &tn->geometry();
        if (area) {
            std::cout << " - looking in area: " << *area << " " << e->position() << " " << area->contains(e->position()) << std::endl;
            if (area->contains(e->position()) && onPointerEvent(node, e)) {
                std::cout << " -- delivered to: " << node << std::endl;
                return true;
            }
        }
    }

    // Restore the old position
    if (type == Node::TransformNodeType)
        e->setPosition(pos);

    return false;
}

RENGINE_END_NAMESPACE
