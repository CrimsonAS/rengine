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

#include <assert.h>
#include <vector>
#include <functional>

RENGINE_BEGIN_NAMESPACE

template <typename ...Arguments> class Signal;

template <typename ... Arguments>
class SignalHandler
{
public:
    virtual ~SignalHandler() { }
    virtual void onSignal(Arguments ...args) = 0;

    class Function : public SignalHandler<Arguments ...>
    {
    public:
        Function(const std::function<void(Arguments ...)> handler) : m_handler(handler) { }
        void onSignal(Arguments ... args) override { m_handler(args...); }
    private:
        std::function<void(Arguments ...)> m_handler;
    };

};



template <typename ...Arguments>
class Signal
{
public:

    ~Signal()
    {
        delete m_handlers;
    }

    void emit(Arguments...args) {
        if (m_handlers)
            for(auto handler : *m_handlers)
                handler->onSignal(args...);
    }

    void connect(SignalHandler<Arguments...> *handler) {
        if (!m_handlers)
            m_handlers = new std::vector<SignalHandler<Arguments...> *>();
        m_handlers->push_back(handler);
    }

    void disconnect(SignalHandler<Arguments...> *handler) {
        assert(m_handlers);
        auto pos = std::find(m_handlers->begin(), m_handlers->end(), handler);
        assert(pos != m_handlers->end());
        m_handlers->erase(pos);
    }

private:
    std::vector<SignalHandler<Arguments...> *> *m_handlers = nullptr;
};

RENGINE_END_NAMESPACE