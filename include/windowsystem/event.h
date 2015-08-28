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

class Event 
{
public:
	enum Type {
        PointerDown,
		PointerUp,
        PointerMove
	};

    Event(Type t) : m_type(t) { }
    virtual ~Event() {}

    inline Type type() const { return m_type; }

private:
	Type m_type;
};


class PointerEvent : public Event
{
public:
    PointerEvent(Type t) : Event(t) {
        assert(t == PointerUp 
               || t == PointerDown
               || t == PointerMove);
    };

    void setPosition(const vec2 &position) { m_pos = position; }
	vec2 position() const { return m_pos; }

    static PointerEvent *from(Event *e) {
        assert(e->type() == PointerUp || e->type() == PointerDown || e->type() == PointerMove);
        return static_cast<PointerEvent *>(e);
    }

private:
    vec2 m_pos;
};

RENGINE_END_NAMESPACE