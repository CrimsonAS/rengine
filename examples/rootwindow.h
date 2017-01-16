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

class Button;
class Example;

class RootWindow : public rengine::StandardSurface
{
public:
    rengine::Node *build() override;

    void startExample(Example *example);
    void closeExample();
    void exampleClosed();

private:
    void updateHoverTarget(rengine::Node *node);
    void add(Example *example, const rengine::Units &units);
    Button *createTextButton(const char *name, const rengine::Units &units);
    void animateButtons(Example *example, float from, float to, float initialDelay);

    rengine::GlyphContext *m_font = nullptr;

    rengine::OpacityNode *m_appLayer = nullptr;
    Button *m_closeButton = nullptr;

    rengine::LayoutNode *m_buttonGrid = nullptr;
    std::vector<Example *> m_examples;
    Example *m_example = nullptr;

    std::vector<rengine::SignalHandler<> *> m_buttonSignalHandlers;
};