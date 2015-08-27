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

template <typename T>
class Property {
public:

    Property(const T &t = T()) : m_t(t) { }

    const T &get() const { return m_t; }
    void set(const T &t) {
        if (t == m_t)
            return;
        m_t = t;
        for (auto callback : m_listeners)
            callback();
    }

    operator const T &() const { return get(); }
    void operator=(const T &t) { set(t); }

    bool operator==(const T &t) const { return m_t == t; }

    int connect(const std::function<void()> &listener) {
        m_listeners.push_back(listener);
        return m_listeners.size() - 1;
    }

    void disconnect(unsigned id) {
        assert(id < m_listeners.size());
        m_listeners.erase(m_listeners.begin() + id);
    }

private:
    T m_t;

    std::vector<std::function<void()> > m_listeners;
};

template <typename T>
class BoundedProperty : public Property<T>
{
public:

    static const T &clamp(const T &min, const T &max, const T &value) {
        return (value < min ? min : (value > max ? max : value));
    }

    BoundedProperty(const T &min, const T &max, const T &value = T())
        : Property<T>(clamp(min, max, value))
        , m_min(min)
        , m_max(max)
    {
    }

    void set(const T &v) { Property<T>::set(clamp(m_min, m_max, v)); }
    void operator=(const T &v) { set(v); }

private:
    T m_min;
    T m_max;
};



RENGINE_END_NAMESPACE