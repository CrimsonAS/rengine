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

#include "example.h"
#include "imageutils.h"

class BlurExample : public Example
{
public:
    const char *name() const override { return "Blur"; }

    void initialize() override {
        m_texture.reset(ImageUtils::load(renderer(), "walker.png"));

        vec2 s = size();
        float w2 = s.x / 2.0f;
        float h2 = s.y / 2.0f;
        float s2 = std::min(w2, h2);

        BlurNode *blurNode = BlurNode::create(37);

        auto a = make_shared<Animation_BlurNode_radius>(blurNode);
        a->setDuration(2);
        a->setDirection(AbstractAnimation::Alternate);
        a->setIterations(-1);
        a->newKeyFrame(0) = 0;
        a->newKeyFrame(1) = 100;

        animationManager()->start(a);
        m_animation = a;

        mat4 matrix = mat4::translate2D(w2, h2)
                      * mat4::scale2D(s2, s2);
        Node *root = &(*TransformNode::create(matrix)
                    << &(*blurNode
                       << RectangleNode::create(rect2d::fromXywh(0.15, -0.5,  0.3, 0.3), vec4(1, 0, 0, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.15, -0.15, 0.3, 0.3), vec4(0, 1, 0, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.15,  0.2,  0.3, 0.3), vec4(0, 0, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh( 0.5, -0.5,  0.3, 0.3), vec4(1, 1, 0, 1))
                       << RectangleNode::create(rect2d::fromXywh( 0.5, -0.15, 0.3, 0.3), vec4(0, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh( 0.5,  0.2,  0.3, 0.3), vec4(1, 0, 1, 1))
                       << TextureNode::create(rect2d::fromXywh(-0.8, -0.5, 0.65, 1.0), m_texture.get())
                     )
                );
        append(root);
    }

    void invalidate() override {
        animationManager()->stop(m_animation);
        m_animation.reset();
        m_texture.reset();
        destroyAllChildren();
    }

    AnimationPtr m_animation;
    std::unique_ptr<Texture> m_texture;

};
