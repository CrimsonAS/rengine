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

/*!
    Q: Why duplicate this? There are plenty of libraries which provide basic math?

    A: True, but I want this library and this file in particular to be a
    selfcontained, no-hassle-to-deploy suite of strictly needed math things.
    It isn't complete or perhaps not fully optimal either (due to no sse and such),
    but it is all inline, comes at no deployment cost and the file can be included
    in any project by simply dumping a single header file in there.
 */

#pragma once

#include <math.h>
#include <ostream>

RENGINE_BEGIN_NAMESPACE

struct vec2 {

    vec2(float x, float y) : x(x), y(y) { }
    vec2(float v = 0.0f) : x(v), y(v) { }

    vec2 operator*(float v) const { return vec2(x*v, y*v); }
    vec2 operator/(float v) const { return vec2(x/v, y/v); }
    vec2 operator+(const vec2 &o) const { return vec2(x+o.x, y+o.y); }
    vec2 operator-(const vec2 &o) const { return vec2(x-o.x, y-o.y); }
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
    bool operator==(const vec2 &o) const {
        return o.x == x && o.y == y;
    }

    float x;
    float y;
};

struct vec3 {

    vec3(float x, float y, float z = 0) : x(x), y(y), z(z) { }
    vec3(float v = 0.0f) : x(v), y(v), z(v) { }
    vec3(const vec2 &v, float z = 0) : x(v.x), y(v.y), z(z) { }

    vec2 project2D(float farPlane) const {
        float zScale = (farPlane - z) / farPlane;
        return vec2(x / zScale, y / zScale);
    }

    vec3 operator*(float v) const { return vec3(x*v, y*v, z*v); }
    vec3 operator/(float v) const { return vec3(x/v, y/v, z/v); }
    vec3 operator+(const vec3 &o) const { return vec3(x+o.x, y+o.y, z+o.z); }
    vec3 operator-(const vec3 &o) const { return vec3(x-o.x, y-o.y, z+o.z); }
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
    bool operator==(const vec3 &o) const {
        return o.x == x && o.y == y && o.z == z;
    }

    float x;
    float y;
    float z;
};

struct vec4 {

    vec4(float x, float y, float z=0, float w=0) : x(x), y(y), z(z), w(w) { }
    vec4(float v = 0.0f) : x(v), y(v), z(v), w(v) { }
    vec4(const vec2 &v, float z = 0, float w = 0) : x(v.x), y(v.y), z(z), w(w) { }
    vec4(const vec3 &v, float w = 0) : x(v.x), y(v.y), z(v.z), w(w) { }

    vec4 operator*(float v) const { return vec4(x*v, y*v, z*v, w*v); }
    vec4 operator/(float v) const { return vec4(x/v, y/v, z/v, w/v); }
    vec4 operator+(const vec4 &o) const { return vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
    vec4 operator-(const vec4 &o) const { return vec4(x-o.x, y-o.y, z-o.z, w-o.w); }
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
    bool operator==(const vec4 &o) const {
        return o.x == x && o.y == y && o.z == z && o.w == w;
    }

    float x;
    float y;
    float z;
    float w;
};

struct mat4 {
    enum Type {
        Identity         = 0x00,
        Translation2D    = 0x01,
        Scale2D          = 0x02,
        Rotation2D       = 0x04,
        ScaleAndRotate2D = 0x07, // all of the above
        Generic          = 0xff,
    };

    mat4()
        : m{ 1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 }
        , type(Identity)
    {
    }

    mat4(float m11, float m12, float m13, float m14,
         float m21, float m22, float m23, float m24,
         float m31, float m32, float m33, float m34,
         float m41, float m42, float m43, float m44,
         unsigned type = Generic)
        : m{ m11, m12, m13, m14,
             m21, m22, m23, m24,
             m31, m32, m33, m34,
             m41, m42, m43, m44 }
        , type(type)
    {
    }

    bool operator==(const mat4 &o) const {
        return    m[ 0] == o.m[ 0]
               && m[ 1] == o.m[ 1]
               && m[ 2] == o.m[ 2]
               && m[ 3] == o.m[ 3]
               && m[ 4] == o.m[ 4]
               && m[ 5] == o.m[ 5]
               && m[ 6] == o.m[ 6]
               && m[ 7] == o.m[ 7]
               && m[ 8] == o.m[ 8]
               && m[ 9] == o.m[ 9]
               && m[10] == o.m[10]
               && m[11] == o.m[11]
               && m[12] == o.m[12]
               && m[13] == o.m[13]
               && m[14] == o.m[14]
               && m[15] == o.m[15];
    }

    mat4 operator*(const mat4 &o) const {

        if (type == Translation2D && o.type == Translation2D) {
            return mat4(1, 0, 0, m[3]+o.m[3],
                        0, 1, 0, m[7]+o.m[7],
                        0, 0, 1, 0,
                        0, 0, 0, 1,
                        Translation2D);

        } else if (type == Translation2D) {
            return mat4(o.m[ 0] + m[ 3] * o.m[12],
                        o.m[ 1] + m[ 3] * o.m[13],
                        o.m[ 2] + m[ 3] * o.m[14],
                        o.m[ 3] + m[ 3] * o.m[15],
                        o.m[ 4] + m[ 7] * o.m[12],
                        o.m[ 5] + m[ 7] * o.m[13],
                        o.m[ 6] + m[ 7] * o.m[14],
                        o.m[ 7] + m[ 7] * o.m[15],
                        o.m[ 8],
                        o.m[ 9],
                        o.m[10],
                        o.m[11],
                        o.m[12],
                        o.m[13],
                        o.m[14],
                        o.m[15],
                        type | o.type);

        } else if (o.type == Translation2D) {
            return mat4(m[ 0], m[ 1], m[ 2], m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 3],
                        m[ 4], m[ 5], m[ 6], m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 7],
                        m[ 8], m[ 9], m[10], m[ 8] * o.m[3] + m[ 9] * o.m[7] + m[11],
                        m[12], m[13], m[14], m[12] * o.m[3] + m[13] * o.m[7] + m[15],
                        Type(type | o.type));

        } else if (type <= ScaleAndRotate2D && o.type <= ScaleAndRotate2D) {
            return mat4(m[ 0] * o.m[0] + m[ 1] * o.m[4],
                        m[ 0] * o.m[1] + m[ 1] * o.m[5],
                        0,
                        m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 3],

                        m[ 4] * o.m[0] + m[ 5] * o.m[4],
                        m[ 4] * o.m[1] + m[ 5] * o.m[5],
                        0,
                        m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 7],

                        0, 0, 1, 0,
                        0, 0, 0, 1,
                        Type(type | o.type)) ;
            }

        // Genereic full multiplication
        return mat4(m[ 0] * o.m[0] + m[ 1] * o.m[4] + m[ 2] * o.m[ 8] + m[ 3] * o.m[12],
                    m[ 0] * o.m[1] + m[ 1] * o.m[5] + m[ 2] * o.m[ 9] + m[ 3] * o.m[13],
                    m[ 0] * o.m[2] + m[ 1] * o.m[6] + m[ 2] * o.m[10] + m[ 3] * o.m[14],
                    m[ 0] * o.m[3] + m[ 1] * o.m[7] + m[ 2] * o.m[11] + m[ 3] * o.m[15],

                    m[ 4] * o.m[0] + m[ 5] * o.m[4] + m[ 6] * o.m[ 8] + m[ 7] * o.m[12],
                    m[ 4] * o.m[1] + m[ 5] * o.m[5] + m[ 6] * o.m[ 9] + m[ 7] * o.m[13],
                    m[ 4] * o.m[2] + m[ 5] * o.m[6] + m[ 6] * o.m[10] + m[ 7] * o.m[14],
                    m[ 4] * o.m[3] + m[ 5] * o.m[7] + m[ 6] * o.m[11] + m[ 7] * o.m[15],

                    m[ 8] * o.m[0] + m[ 9] * o.m[4] + m[10] * o.m[ 8] + m[11] * o.m[12],
                    m[ 8] * o.m[1] + m[ 9] * o.m[5] + m[10] * o.m[ 9] + m[11] * o.m[13],
                    m[ 8] * o.m[2] + m[ 9] * o.m[6] + m[10] * o.m[10] + m[11] * o.m[14],
                    m[ 8] * o.m[3] + m[ 9] * o.m[7] + m[10] * o.m[11] + m[11] * o.m[15],

                    m[12] * o.m[0] + m[13] * o.m[4] + m[14] * o.m[ 8] + m[15] * o.m[12],
                    m[12] * o.m[1] + m[13] * o.m[5] + m[14] * o.m[ 9] + m[15] * o.m[13],
                    m[12] * o.m[2] + m[13] * o.m[6] + m[14] * o.m[10] + m[15] * o.m[14],
                    m[12] * o.m[3] + m[13] * o.m[7] + m[14] * o.m[11] + m[15] * o.m[15],
                    Type(type | o.type)) ;
    }

    vec2 operator*(const vec2 &v) const {
        return vec2(m[0] * v.x + m[1] * v.y + m[3],
                    m[4] * v.x + m[5] * v.y + m[7]);
    }

    vec3 operator*(const vec3 &v) const {
        return vec3(m[0] * v.x + m[1] * v.y + m[ 2] * v.z + m[ 3],
                    m[4] * v.x + m[5] * v.y + m[ 6] * v.z + m[ 7],
                    m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]);
    }

    vec4 operator*(const vec4 &v) const {
        return vec4(m[ 0] * v.x + m[ 1] * v.y + m[ 2] * v.z + m[ 3] * v.w,
                    m[ 4] * v.x + m[ 5] * v.y + m[ 6] * v.z + m[ 7] * v.w,
                    m[ 8] * v.x + m[ 9] * v.y + m[10] * v.z + m[11] * v.w,
                    m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w);
    }


    static mat4 translate2D(float dx, float dy) {
        return mat4(1, 0, 0, dx,
                    0, 1, 0, dy,
                    0, 0, 1, 0,
                    0, 0, 0, 1, Translation2D);
    }


    static mat4 scale2D(float sx, float sy) {
        return mat4(sx,  0,  0, 0,
                     0, sy,  0, 0,
                     0,  0,  1, 0,
                     0,  0,  0, 1, Scale2D);
    }

    static mat4 rotate2D(float radians) {
        float s = sin(radians);
        float c = cos(radians);
        return mat4(c, -s, 0, 0,
                    s,  c, 0, 0,
                    0,  0, 1, 0,
                    0,  0, 0, 1, Rotation2D);
    }

    static mat4 translate(float dx, float dy, float dz) {
        return mat4(1, 0, 0, dx,
                    0, 1, 0, dy,
                    0, 0, 1, dz,
                    0, 0, 0, 1, Translation2D);
    }

    static mat4 rotateAroundZ(float radians) { return rotate2D(radians); }

    static mat4 rotateAroundX(float radians) {
        float s = sin(radians);
        float c = cos(radians);
        return mat4(1, 0,  0, 0,
                    0, c, -s, 0,
                    0, s,  c, 0,
                    0, 0,  0, 1, Generic);
    }

    static mat4 rotateAroundY(float radians) {
        float s = sin(radians);
        float c = cos(radians);
        return mat4( c, 0,  s, 0,
                     0, 1,  0, 0,
                    -s, 0,  c, 0,
                     0, 0,  0, 1, Generic);
    }

    static mat4 scale(float sx, float sy, float sz) {
        return mat4(sx,  0,  0, 0,
                     0, sy,  0, 0,
                     0,  0, sz, 0,
                     0   ,   0, 1, Generic);
    }

    bool isIdentity() const { return type == Identity; }

    float operator()(int c, int r) {
        return m[c*4 + r];
    }

    float m[16];
    unsigned type;

};

inline std::ostream &operator<<(std::ostream &o, const vec2 &v) {
    o << "vec2(" << v.x << ", " << v.y << ")";
    return o;
}

inline std::ostream &operator<<(std::ostream &o, const vec3 &v) {
    o << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return o;
}

inline std::ostream &operator<<(std::ostream &o, const vec4 &v) {
    o << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return o;
}

inline std::ostream &operator<<(std::ostream &o, const mat4 &m) {
    o << "mat4(" << m.m[0];
    for (int i=1; i<16; ++i)
        o << ", " << m.m[i];
    o << ")";
    return o;
}

RENGINE_END_NAMESPACE
