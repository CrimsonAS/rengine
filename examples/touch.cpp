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

#include "examples.h"

const vec4 COLOR_IDLE(0.5, 0.6, 0.8, 1.0);
const vec4 COLOR_ACTIVE(0.8, 0.6, 0.5, 1.0);

class TouchWindow : public StandardSurfaceInterface
{
public:

    Node *update(Node *root) override {
        if (root)
            return root;

        root = Node::create();

        vec2 surfaceSize = surface()->size();

        float size = std::min(surfaceSize.x, surfaceSize.y);

        vec2 c = surfaceSize / 2.0;
        float boxSize = size * 0.3;
        float shift = boxSize + size * 0.01;

        m_left  = RectangleNode::create(rect2d::fromXywhCentered(c.x - shift, c.y, boxSize, boxSize), COLOR_IDLE);
        m_right = RectangleNode::create(rect2d::fromXywhCentered(c.x + shift, c.y, boxSize, boxSize), COLOR_IDLE);

        root->append(m_left);
        root->append(m_right);

        return root;
    }

    void activate(RectangleNode *r) {
        assert(!m_currentReceiver);
        m_currentReceiver = r;
        m_currentReceiver->setColor(COLOR_ACTIVE);
        surface()->requestRender();
    }

    void deactivate() {
        assert(m_currentReceiver);
        m_currentReceiver->setColor(COLOR_IDLE);
        m_currentReceiver = 0;
        surface()->requestRender();
    }

    void dispatchEvent(Event *e) override {
        if (e->type() == Event::PointerDown) {
            vec2 p = PointerEvent::from(e)->position();
            if (m_left->geometry().contains(p)) 
                activate(m_left);
            else if (m_right->geometry().contains(p))
                activate(m_right);
        } else if (m_currentReceiver && e->type() == Event::PointerUp) {
            deactivate();
        }
    }

private:
    RectangleNode *m_left  = nullptr;
    RectangleNode *m_right = nullptr;
    RectangleNode *m_currentReceiver = nullptr;
};

RENGINE_MAIN(TouchWindow);