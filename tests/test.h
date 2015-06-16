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

#ifdef NDEBUG
    #undef NDEBUG
#endif
#include <assert.h>
#include <cmath>
#include <iostream>
#include "rengine.h"

using namespace std;

RENGINE_BEGIN_NAMESPACE

inline bool fuzzy_equals(float a, float b, float threshold = 0.0001f) { return abs(a - b) < threshold; }

inline bool fuzzy_equals(const vec2 &a, const vec2 &b, float threshold = 0.0001f) {
    return fuzzy_equals(a.x, b.x, threshold)
           && fuzzy_equals(a.y, b.y, threshold);
}

inline bool fuzzy_equals(const vec3 &a, const vec3 &b, float threshold = 0.0001f) {
    return fuzzy_equals(a.x, b.x, threshold)
           && fuzzy_equals(a.y, b.y, threshold)
           && fuzzy_equals(a.z, b.z, threshold);
}
inline bool fuzzy_equals(const vec4 &a, const vec4 &b, float threshold = 0.0001f) {
    return fuzzy_equals(a.x, b.x, threshold)
           && fuzzy_equals(a.y, b.y, threshold)
           && fuzzy_equals(a.z, b.z, threshold)
           && fuzzy_equals(a.w, b.w, threshold);
}

#define check_true(cond)                       \
    if (!(cond)) {                               \
        assert(cond);                          \
    }

#define check_equal(a, b)                                             \
    if (!((a) == (b))) {                                                  \
        cout << "Not equal '" << a << "' vs '" << b << "'" << endl;   \
        assert((a) == (b));                                               \
    }

#define check_equal_hex(a, b)                                             \
    if (!((a) == (b))) {                                                  \
        cout << "Not equal '" << hex << a << "' vs '" << b << "'" << endl;   \
        assert((a) == (b));                                               \
    }

#define check_fuzzyEqual(a, b)                                              \
    if (!fuzzy_equals(a, b)) {                                              \
        cout << "Not fuzzy equal '" << a << "' vs '" << b << "'" << endl;   \
        assert((a) == (b));                                                     \
    }

class StaticRenderTest {
public:
    virtual Node *build() = 0;
    virtual bool check() = 0;

    bool checkPixel(int x, int y, const vec4 &expected, float errorMargin=0.01)
    {
        assert(x >= 0);
        assert(x < m_w);
        assert(y >= 0);
        assert(y < m_h);

        unsigned pixel = m_pixels[(m_h - y - 1) * m_w + x];
        vec4 color = vec4((pixel & 0x000000ff) >> 0,
                          (pixel & 0x0000ff00) >> 8,
                          (pixel & 0x00ff0000) >> 16,
                          (pixel & 0xff000000) >> 24) / 255.0;

        if (!fuzzy_equals(color, expected, errorMargin)) {
            cout << "pixels differ: (" << x << "," << y << ")=" << color << "; expected=" << expected << endl;
            assert(false);
        }

        return true;
    }

    Surface *surface() const { return m_surface; }
    void setSurface(Surface *surface) { m_surface = surface; }

    void setPixels(int w, int h, unsigned *pixels) {
        m_w = w;
        m_h = h;
        m_pixels = pixels;
    }

    virtual const char *name() const = 0;

private:
    int m_w;
    int m_h;
    unsigned *m_pixels;
    Surface *m_surface;
};

class TestBase : public StandardSurfaceInterface
{
public:
    TestBase() : leaveRunning(false), m_currentTest(0) { }

    Node *update(Node *root) {
        if (root)
            delete root;

        m_currentTest = tests.front();
        tests.pop_front();

        m_currentTest->setSurface(surface());

        return m_currentTest->build();
    }

    void afterRender() override {
        vec2 size = surface()->size();
        unsigned *pixels = (unsigned *) malloc(size.x * size.y * sizeof(unsigned));

        bool ok = renderer()->readPixels(0, 0, size.x, size.y, (unsigned char *) pixels);
        check_true(ok);

        m_currentTest->setPixels(size.x, size.y, pixels);
        if (m_currentTest->check()) {
            cout << "testng: '" << m_currentTest->name() << "': ok" << endl;
        } else {
            cout << m_currentTest->name() << ": failed!" << endl;
        }

        if (tests.empty()) {
            if (!leaveRunning)
                Backend::get()->quit();
        } else {
            surface()->requestRender();
        }
    }

    bool leaveRunning;

    StaticRenderTest *m_currentTest;
    list<StaticRenderTest *> tests;
};


RENGINE_END_NAMESPACE
RENGINE_USE_NAMESPACE

