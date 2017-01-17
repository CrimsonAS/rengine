/*
    Copyright (c) 2017, Gunnar Sletta <gunnar@crimson.no>
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

#include "rengine.h"

#include "example.h"

using namespace rengine;
using namespace std;

class LayeredOpacity : public Example
{
public:
    const char *name() const override { return "Layered Opacity"; }

    void initialize() override {
        vec2 s = size();
        vec2 s3 = s / 3;

        // Root has origin in screen center
        TransformNode *root = TransformNode::create();
        root->setMatrix(mat4::translate2D(s.x * 0.5, s.y * 0.5));

        OpacityNode *opacityNode = OpacityNode::create();
        root->append(opacityNode);

        vec4 color(0.5, 0.5, 0.8, 1.0);

        *opacityNode << RectangleNode::create(rect2d::fromPosSize(-s3, s3), color)
                     << RectangleNode::create(rect2d::fromPosSize(-s3/2, s3), color)
                     << RectangleNode::create(rect2d::fromPosSize(vec2(), s3), color);

        auto anim = std::make_shared<Animation_OpacityNode_opacity>(opacityNode);
        anim->setDuration(3);
        anim->setDirection(AbstractAnimation::Alternate);
        anim->setIterations(-1);
        anim->newKeyFrame(0) = 1;
        anim->newKeyFrame(1) = 0;

        animationManager()->start(anim);

        m_animation = anim;

        append(root);
    }

    void invalidate() override {
        animationManager()->stop(m_animation);
        m_animation.reset();
        destroyAllChildren();
    }

    AnimationPtr m_animation;
};
