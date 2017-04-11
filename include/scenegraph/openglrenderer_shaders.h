/*
 * Copyright (c) 2017 Crimson AS <info@crimson.no>
 * Author: Gunnar Sletta <gunnar@crimson.no>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "opengl.h"

inline const char *openglrenderer_vsh_solid() { return RENGINE_GLSL(
   attribute highp vec2 aV;
   uniform highp mat4 m;
   void main() {
       gl_Position = m * vec4(aV, 0, 1);
   }
); }

inline const char *openglrenderer_fsh_solid() { return RENGINE_GLSL(
    uniform lowp vec4 color;
    void main() {
        gl_FragColor = color;
    }
); }

inline const char *openglrenderer_vsh_texture() { return RENGINE_GLSL(
    attribute highp vec2 aV;
    attribute highp vec2 aT;
    uniform highp mat4 m;
    varying highp vec2 vT;
    void main() {
        gl_Position = m * vec4(aV, 0, 1);
        vT = aT;
    }
); }

inline const char *openglrenderer_fsh_texture() { return RENGINE_GLSL(
    uniform lowp sampler2D t;
    varying highp vec2 vT;
    void main() {
        gl_FragColor = texture2D(t, vT);
    }
); }

inline const char *openglrenderer_fsh_texture_bgra() { return RENGINE_GLSL(
    uniform lowp sampler2D t;
    varying highp vec2 vT;
    void main() {
        gl_FragColor = texture2D(t, vT).zyxw;
    }
); }

inline const char *openglrenderer_fsh_texture_alpha() { return RENGINE_GLSL(
    uniform lowp sampler2D t;
    uniform lowp float alpha;
    varying highp vec2 vT;
    void main() {
        gl_FragColor = texture2D(t, vT) * alpha;
    }
); }

inline const char *openglrenderer_fsh_texture_colorfilter() { return RENGINE_GLSL(
    uniform lowp sampler2D t;
    uniform lowp mat4 CM;
    varying highp vec2 vT;
    void main() {
        gl_FragColor = CM * texture2D(t, vT);
    }
); }

// ### Naive implementation with a lot of room for improvement...
//
// Compatibility wise, there are several older and lower-end chips that do not
// support using a uniform in a loop condition. This is not mandated by the
// GLSL spec, so it won't work everywhere.
inline const char *openglrenderer_vsh_blur() { return RENGINE_GLSL(
    attribute highp vec2 aV;
    attribute highp vec2 aT;
    uniform highp mat4 m;
    uniform int radius;
    uniform highp vec4 dims;
    varying highp vec2 vT;
    void main() {
        gl_Position = m * vec4(aV, 0, 1);
        highp vec2 aw = dims.xy;
        highp vec2 cw = dims.zw;
        highp vec2 diff = (aw - cw) / aw;
        vT = (aT - diff/2.0) * (aw / cw);
    }
); }

inline const char *openglrenderer_fsh_blur() { return RENGINE_GLSL(
    uniform lowp sampler2D t;
    uniform highp vec4 dims;
    uniform highp vec2 step;
    uniform highp float sigma;
    uniform int radius;
    varying highp vec2 vT;
    highp float gauss(float x) { return exp(-(x*x)/sigma); }
    void main() {
        highp float r = float(radius);
        highp float weights = 0.5 * gauss(r);
        highp vec4 result = weights * texture2D(t, vT - float(radius) * step);
        for (int i=-radius+1; i<=radius; i+=2) {
            highp float p1 = float(i);
            highp float w1 = gauss(p1);
            highp float p2 = float(i+1);
            highp float w2 = gauss(p2);
            highp float w = w1 + w2;
            highp float p = (p1 * w1 + p2 * w2) / w;
            result += w * texture2D(t, vT + p * step);
            weights += w;
        }
        gl_FragColor = result / weights;
    }
); }

inline const char *openglrenderer_fsh_shadow() { return RENGINE_GLSL(
    uniform lowp sampler2D t;
    uniform highp vec4 color;
    uniform highp vec4 dims;
    uniform highp vec2 step;
    uniform highp float sigma;
    uniform int radius;
    varying highp vec2 vT;
    highp float gauss(float x) { return exp(-(x*x)/sigma); }
    void main() {
        highp float r = float(radius);
        highp float weights = 0.5 * gauss(r);
        highp float result = weights * texture2D(t, vT - float(radius) * step).a;
        for (int i=-radius+1; i<=radius; i+=2) {
            highp float p1 = float(i);
            highp float w1 = gauss(p1);
            highp float p2 = float(i+1);
            highp float w2 = gauss(p2);
            highp float w = w1 + w2;
            highp float p = (p1 * w1 + p2 * w2) / w;
            result += w * texture2D(t, vT + p * step).a;
            weights += w;
        }
        gl_FragColor = color * (result / weights);
    }
); }

