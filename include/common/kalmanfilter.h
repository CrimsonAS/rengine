/*
    Copyright (c) 2016, Gunnar Sletta <gunnar@sletta.org>
    Copyright (c) 2016, Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
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

struct KalmanFilter2D
{
    KalmanFilter2D();

    void initialize(float pos, float velocity);
    void update(float pos, float velocity, float timeDelta);

    float position() const { return x.x; }
    float velocity() const { return x.y; }

private:
    struct mat2 {
        float a, b, c, d;
        mat2(float a = 1.0f, float b = 0.0f, float c = 0.0f, float d = 1.0f)
            : a(a)
            , b(b)
            , c(c)
            , d(d)
        {
        }

        mat2 transposed() const {
            return mat2(a, c,
                        b, d);
        }

        mat2 inverted() const {
            float det = 1.0f / (a * d - b * c);
            return mat2( d * det, -b * det,
                        -c * det,  a * det);
        }

        mat2 operator+(mat2 m) const {
            return mat2(a + m.a, b + m.b,
                        c + m.c, d + m.d);
        }

        mat2 operator-(mat2 m) const {
            return mat2(a - m.a, b - m.b,
                        c - m.c, d - m.d);
        }

        vec2 operator*(vec2 v) const {
            return vec2(a * v.x + b * v.y,
                        c * v.x + d * v.y);
        }

        mat2 operator*(mat2 M) const {
            return mat2(a * M.a + b * M.c,
                        a * M.b + b * M.d,
                        c * M.a + d * M.c,
                        c * M.b + d * M.d);
        }
    };

    vec2 x;
    mat2 A;
    mat2 P;
    mat2 Q;
    mat2 R;
    mat2 H;
};

inline KalmanFilter2D::KalmanFilter2D()
{
}

inline void KalmanFilter2D::initialize(float pos, float velocity)
{
    x = vec2(pos, velocity);

    P = mat2(0.0f, 0.0f,
             0.0f, 0.0f);

    Q = mat2(0.0f, 0.0f,
             0.0f, 0.1f);
    R = mat2(0.1f, 0.0f,
             0.0f, 0.1f);
}

inline void KalmanFilter2D::update(float pos, float velocity, float dT)
{
    A.b = dT;

    // Prediction setp
    x = A * x;
    P = A * P * A.transposed() + Q;

    // Correction step (complete with H)
    // mat2 S = H * P * H.transposed() + R;
    // mat2 K = P * H.transposed() * S.inverted();
    // vec2 m(pos, velocity);
    // vec2 y = m - H * x;
    // x = x + K * y;
    // P = (mat2() - K * H) * P;

    // Correction step (without H as H is currently set to I, so we can ignore
    // it in the calculations...)
    mat2 S = P + R;
    mat2 K = P * S.inverted();
    vec2 m(pos, velocity);
    vec2 y = m - x;
    x = x + K * y;
    P = (mat2() - K) * P;

}

RENGINE_END_NAMESPACE
