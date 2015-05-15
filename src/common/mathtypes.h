#pragma once

namespace rengine {

struct vec2 {
    float x;
    float y;

    vec2(float x, float y) : x(x), y(y) { }
    vec2(float v) : x(v), y(v) { }

    vec2 operator+(const vec2 &o) const {
        vec2 r = o;
        r.x += x;
        r.y += y;
        return r;
    }
    vec2 &operator+=(const vec2 &o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    vec2 &operator=(float v) {
        x = v;
        y = v;
        return *this;
    }
};

struct vec3 {
    float x;
    float y;
    float z;

    vec3(float x, float y, float z) : x(x), y(y), z(z) { }
    vec3(float v) : x(v), y(v), z(v) { }

    vec3 operator+(const vec3 &o) const {
        vec3 r = o;
        r.x += x;
        r.y += y;
        r.z += z;
        return r;
    }
    vec3 &operator+=(const vec3 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    vec3 &operator=(float v) {
        x = v;
        y = v;
        z = v;
        return *this;
    }
};

struct vec4 {
    float x;
    float y;
    float z;
    float w;

    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
    vec4(float v) : x(v), y(v), z(v), w(v) { }

    vec4 operator+(const vec4 &o) const {
        vec4 r = o;
        r.x += x;
        r.y += y;
        r.z += z;
        r.w += w;
        return r;
    }
    vec4 &operator+=(const vec4 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        w += o.w;
        return *this;
    }
    vec4 &operator=(float v) {
        x = v;
        y = v;
        z = v;
        w = v;
        return *this;
    }
};

}
