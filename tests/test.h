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

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
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
    inline bool fuzzy_equals(float a, float b) { return abs(a - b) < 0.0001f; }

    inline bool fuzzy_equals(const vec2 &a, const vec2 &b) {
        return fuzzy_equals(a.x, b.x)
               && fuzzy_equals(a.y, b.y);
    }

    inline bool fuzzy_equals(const vec3 &a, const vec3 &b) {
        return fuzzy_equals(a.x, b.x)
               && fuzzy_equals(a.y, b.y)
               && fuzzy_equals(a.z, b.z);
    }
    inline bool fuzzy_equals(const vec4 &a, const vec4 &b) {
        return fuzzy_equals(a.x, b.x)
               && fuzzy_equals(a.y, b.y)
               && fuzzy_equals(a.z, b.z)
               && fuzzy_equals(a.w, b.w);
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

    #define check_fuzzyEqual(a, b)                                              \
        if (!fuzzy_equals(a, b)) {                                              \
            cout << "Not fuzzy equal '" << a << "' vs '" << b << "'" << endl;   \
            assert((a) == (b));                                                     \
        }
RENGINE_END_NAMESPACE
RENGINE_USE_NAMESPACE

