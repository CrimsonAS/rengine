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

class StandardSurface : public Surface
{
public:
    StandardSurface()
    {
    }

    ~StandardSurface()
    {
        if (m_renderer) {
            if (m_renderer->sceneRoot())
                m_renderer->sceneRoot()->destroy();
        }
    }

    virtual Node *update(Node *oldRoot) = 0;

    virtual void onBeforeRender() { }
    virtual void onAfterRender() { }

    void onRender() override {
        if (!beginRender())
            return;

        // Initialize the renderer if this is the first time around
        if (!m_renderer) {
            m_renderer.reset(createRenderer());
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

        commitRender();
        m_renderer->frameSwapped();

        // Schedule a repaint again if there are animations running...

        // ### TODO: Optimize waiting for scheduled animations. Rather than
        // just keep on rendering, we should figure out how long we need to
        // wait and schedule an update at that time.
        if (m_animationManager.animationsRunning() || m_animationManager.animationsScheduled()) {
            requestRender();
        }
    }

    virtual bool onPointerEvent(Node * /*node*/, PointerEvent * /*event*/) {
        return true;
    }

    virtual void onEvent(Event *e) override;

    Renderer *renderer() const { return m_renderer.get(); }
    AnimationManager *animationManager() { return &m_animationManager; }

    /*!
        Set the pointer event receiver to node to indicate that onPointerEvent() should
        be called with \a node as argument regardless of where the pointer is.
     */
    void setPointerEventReceiver(Node *node) { m_pointerEventReceiver = node; }
    Node *pointerEventReceiver() const { return m_pointerEventReceiver; }

protected:
    bool deliverPointerEventInScene(Node *n, PointerEvent *e);

    std::unique_ptr<Renderer> m_renderer;
    AnimationManager m_animationManager;

    Node *m_pointerEventReceiver = nullptr;
};

inline void StandardSurface::onEvent(Event *e)
{
    switch (e->type()) {
    case Event::PointerDown:
    case Event::PointerUp:
    case Event::PointerMove:
        if (m_renderer && m_renderer->sceneRoot()) {
            PointerEvent *pe = PointerEvent::from(e);
            if (m_pointerEventReceiver) {
                bool inv = false;
                mat4 invNodeMatrix = TransformNode::matrixFor(m_pointerEventReceiver, m_renderer->sceneRoot()).inverted(&inv);
                if (inv)
                    pe->setPosition(invNodeMatrix * pe->positionInSurface());
                else
                    pe->setPosition(vec2());
                onPointerEvent(m_pointerEventReceiver, pe);
            } else {
                deliverPointerEventInScene(m_renderer->sceneRoot(), pe);
            }
        }
        break;
    default:
        std::cerr << __PRETTY_FUNCTION__ << ": unknown event type=" << e->type() << std::endl;
        break;
    }
}

inline bool StandardSurface::deliverPointerEventInScene(Node *node, PointerEvent *e)
{
    assert(node);
    assert(e);

    // Traverse children in backwards order (and bottom up), so we get
    // inverse-paint order delivery
    Node *child = node->lastChild();
    while (child) {
        if (deliverPointerEventInScene(child, e))
            return true;
        child = child->previousSibling();
    }
    if (node->isPointerTarget()) {
        RectangleNodeBase *rectNode = RectangleNodeBase::from(node);
        if (rectNode) {
            rect2d area = rectNode->geometry();
            bool inv = false;
            mat4 nodeInvMatrix = TransformNode::matrixFor(node, m_renderer->sceneRoot()).inverted(&inv);

            // can only be inside if the matrix is invertible, as otherwise
            // the node will be "collapsed" in some dimension
            if (inv) {
                // Note that this doesn't bother to unset the position afterwards as
                // we will either:
                // 1. accept it and the value is correct
                // 2. reject it and the value will be written next time we try..
                e->setPosition(nodeInvMatrix * e->positionInSurface());
                if (area.contains(e->position()) && onPointerEvent(node, e))
                    return true;
            }
        }
    }

    return false;
}

RENGINE_END_NAMESPACE
