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
const vec4 COLOR_CENTER(0.6, 0.8, 0.5, 1.0);

class TouchWindow : public StandardSurface
{
public:

    Node *build() override {
        Node *root = Node::create();

        vec2 surfaceSize = size();

        float size = std::min(surfaceSize.x, surfaceSize.y);

        vec2 c = surfaceSize / 2.0;
        float boxSize = size * 0.4;
        float shift = boxSize + size * 0.01;

        m_left  = RectangleNode::create(rect2d::fromXywhCentered(c.x - shift, c.y, boxSize, boxSize), COLOR_IDLE);
        m_right = RectangleNode::create(rect2d::fromXywhCentered(c.x + shift, c.y, boxSize, boxSize), COLOR_IDLE);
        m_center = RectangleNode::create(rect2d::fromXywhCentered(0, 0, boxSize * 2.0, boxSize / 2), COLOR_CENTER);

        root->append(m_left);
        root->append(m_right);

        TransformNode *xform = TransformNode::create();

        *root << &(*TransformNode::create(mat4::translate2D(c.x, c.y))
                   << &(*xform
                        << m_center)
                  );

        m_left->setPointerTarget(true);
        m_right->setPointerTarget(true);
        m_center->setPointerTarget(true);

        AnimationClosure<TransformNode, SmoothedTimingFunction> *anim = new AnimationClosure<TransformNode, SmoothedTimingFunction>(xform);
        anim->setDuration(10);
        anim->setIterations(-1);
        anim->keyFrames.times() << 0 << 1;
        anim->keyFrames.addValues<double, TransformNode_rotateAroundZ>() << 0 << M_PI * 2.0;
        animationManager()->startAnimation(anim);

        return root;
    }

    void activate(RectangleNode *r) {
        assert(!m_currentReceiver);
        assert(!pointerEventReceiver());
        m_currentReceiver = r;
        m_currentReceiver->setColor(COLOR_ACTIVE);
        requestRender();
        setPointerEventReceiver(m_currentReceiver);
    }

    void deactivate() {
        assert(m_currentReceiver);
        assert(pointerEventReceiver());
        m_currentReceiver->setColor(COLOR_IDLE);
        m_currentReceiver = 0;
        setPointerEventReceiver(0);
        requestRender();
    }

    bool onPointerEvent(Node *n, PointerEvent *event) override {
        if (event->type() == Event::PointerDown) {
            assert(n);
            assert(n == m_left || n == m_right || n == m_center);
            activate(static_cast<RectangleNode *>(n));
            m_pointerPos = event->position();
            return true;
        } else if (m_currentReceiver && event->type() == Event::PointerUp) {
            deactivate();
            return true;
        } else if (event->type() == Event::PointerMove) {
            if (m_currentReceiver == m_left || m_currentReceiver == m_right) {
                vec2 delta = event->position() - m_pointerPos;
                m_pointerPos = event->position();
                RectangleNode *rn = static_cast<RectangleNode *>(m_currentReceiver);
                rn->setPosition(rn->position() + delta);
            }
        }
        return false;
    }

private:
    RectangleNode *m_left  = nullptr;
    RectangleNode *m_right = nullptr;
    RectangleNode *m_center = nullptr;
    RectangleNode *m_currentReceiver = nullptr;

    vec2 m_pointerPos;
};

RENGINE_MAIN(TouchWindow);