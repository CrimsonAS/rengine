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
#include "membersignalhandler.h"

class RootWindow;

class Example : public rengine::OpacityNode
{
public:
    virtual void initialize() = 0;
    virtual void invalidate() = 0;
    virtual const char *name() = 0;

    // Our tie back to the rest of the application..
    RootWindow *window() const { return m_window; }
    void setWindow(RootWindow *window) { m_window = window; }

    // Some convenience
    rengine::vec2 size() const { return m_window->size(); }
    void requestRender() { m_window->requestRender(); }
    rengine::Renderer *renderer() { return m_window->renderer(); }
    rengine::AnimationManager *animationManager() { return m_window->animationManager(); }

    void start();
    void stop();

    void onStopped();

private:
    void scheduleAnimation(float from, float to, float time, float delay);

    RootWindow *m_window = nullptr;
    rengine::AnimationClosure<rengine::OpacityNode, rengine::SmoothedTimingFunction> m_animation = this;
    SignalHandler_Member<Example, &Example::onStopped> handler_onStopped = this;
};

inline void Example::scheduleAnimation(float from, float to, float time, float delay)
{
    std::cout << "Example::scheduleAnimation: " << name()
              << ", from=" << from
              << ", to=" << to
              << ", time=" << time
              << ", delay=" << std::endl;
    m_animation.setDuration(time);
    m_animation.keyFrames = rengine::KeyFrames<OpacityNode>();
    m_animation.keyFrames.times() << 0 << 1;
    m_animation.keyFrames.addValues<double, rengine::OpacityNode_setOpacity>() << from << to;
    animationManager()->scheduleAnimation(delay, &m_animation);
}

inline void Example::start()
{
    std::cout << "Example::start: " << name() << std::endl;
    setOpacity(0);
    scheduleAnimation(0, 1, 0.5, 0.3);
    initialize();
}

inline void Example::stop()
{
    std::cout << "Example::stop: " << name() << std::endl;
    scheduleAnimation(1, 0, 0.5, 0);
    rengine::Animation::onCompleted.connect(&m_animation, &handler_onStopped);
}

inline void Example::onStopped()
{
    std::cout << "Example::onStopped: " << name() << std::endl;
    rengine::Animation::onCompleted.disconnect(&m_animation, &handler_onStopped);
    invalidate();
}
