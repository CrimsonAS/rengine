#pragma once

#include <assert.h>
#include <cmath>
#include "rengine.h"

using namespace rengine;

namespace renginetest
{
    inline bool fuzzy_equals(float a, float b) { return std::abs(a - b) < 0.0001f; }

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


}