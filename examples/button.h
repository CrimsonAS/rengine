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

class Button : public rengine::RectangleNodeBase
{
public:
    enum State {
        DefaultState,
        HoveredState,
        PressedState
    };

    Button(rengine::StandardSurface *surface);
    ~Button();

    void setTextTexture(rengine::Texture *t);

    void scheduleRotation(float from, float to, float time, float delay = 0.0f);

    State state() const { return m_state; }
    void setState(State state);

    static rengine::Signal<> onClicked;

protected:
    void onPreprocess() override;
    bool onPointerEvent(rengine::PointerEvent *event) override;

private:
    rengine::StandardSurface *m_surface;

    std::unique_ptr<rengine::Texture> m_texture;
    rengine::TransformNode *m_xformNode = nullptr;
    rengine::TransformNode *m_offsetNode = nullptr;
    rengine::RectangleNode *m_bgNode = nullptr;
    rengine::TextureNode *m_textureNode = nullptr;

    rengine::AnimationClosure<rengine::TransformNode, rengine::SmoothedTimingFunction> m_animation;

    State m_state = DefaultState;
};