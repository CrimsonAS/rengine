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

#include "rengine.h"
#include <chrono>

// ### Please note that the inclusion of STB_IMAGE_IMPLEMENTATION here forces
// these symbols to be present in all translation units that include the
// example.h header. This will lead to linker errors, so this limits
// the examples to a single .cpp file.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;
using namespace std;

RENGINE_BEGIN_NAMESPACE

typedef Animation<TransformNode, float, &TransformNode::setMatrix_rotateAroundZ, &AnimationCurves::linear> Animation_aroundZ_linear;


inline void animation_rotateZ(AnimationManager *manager, TransformNode *node, float duration)
{
    auto anim = std::make_shared<Animation_aroundZ_linear>(node);
    anim->setDuration(duration);
    anim->setIterations(-1);
    anim->newKeyFrame(0) = 0;
    anim->newKeyFrame(1) = M_PI * 2;
    manager->start(anim);
}

inline void rengine_countFps()
{
    static int frameCounter = 0;
    static std::chrono::steady_clock::time_point then;
    ++frameCounter;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (now > then + std::chrono::milliseconds(int(1000))) {
        double delta = std::chrono::duration<double>(now - then).count();
        cout << "FPS: " << (frameCounter / delta) << endl;
        frameCounter = 0;
        then = now;
    }
}

inline void rengine_fractalTexture(unsigned *bits, vec2 size)
{
    unsigned r = rand() % 256;
    unsigned g = rand() % 256;
    unsigned b = rand() % 256;

    float cy = (rand() % 100) / 100.0;
    float cx = (rand() % 100) / 100.0;
    int iter = 50;

    int w = size.x;
    int h = size.y;
    unsigned stride = w;

    for (int iy=0; iy<h; ++iy) {
        float ty = iy / (float) h;
        for (int ix=0; ix<w; ++ix) {
            float tx = ix / (float) w;

            float zx = 2.0 * (tx - 0.5);
            float zy = 3.0 * (ty - 0.5);
            int i = 0;
            for (i=0; i<iter; ++i) {
                float x = (zx * zx - zy * zy) + cx;
                float y = (zy * zx + zx * zy) + cy;

                if (x*x + y*y > 4.0) break;
                zx = x;
                zy = y;
            }

            float fv = i / float(iter);
            if (fv >= 1.0f)
                fv = 0.0f;
            fv = fv * 3.0f;

            unsigned pa = std::min<unsigned>(255, (fv * 255));
            unsigned pr = std::min<unsigned>(255, (fv * r));
            unsigned pg = std::min<unsigned>(255, (fv * g));
            unsigned pb = std::min<unsigned>(255, (fv * b));
            bits[ix + iy * stride] = (pa << 24) | (pb << 16) | (pg << 8) | (pr);
        }
    }
}

inline Texture *rengine_fractalTexture(Renderer *renderer, vec2 size)
{
    int w = size.x;
    int h = size.y;
    unsigned *bits = new unsigned[w * h];
    rengine_fractalTexture(bits, size);
    Texture *texture = renderer->createTextureFromImageData(size, Texture::RGBA_32, bits);
    delete [] bits;
    return texture;
}

inline Texture *rengine_loadImage(Renderer *renderer, const char *file)
{
    // read the image...
    int w, h, n;
    char location[1024];
    sprintf(location, "../examples/images/%s", file);
    unsigned char *data = stbi_load(location, &w, &h, &n, 4);
    if (!data) {
        sprintf(location, "examples/images/%s", file);
        data = stbi_load(location, &w, &h, &n, 4);
    }
    if (!data) {
        sprintf(location, "%s", file);
        data = stbi_load(location, &w, &h, &n, 4);
    }
    if (!data) {
        cout << "Failed to find the image '" << file << "' under '.' << 'examples/images' or '../examples/images'."
             << "We're a bit dumb, you see, and can't find images unless you've built directly in the "
             << "source directory or in a direct subdirectory, like 'build' or 'debug'..." << endl;
        exit(1);
    }

    // Premultiply it...
    for (int y=0; y<h; ++y)
        for (int x=0; x<w; ++x) {
            unsigned char *p = data + (y * w + x) * 4;
            unsigned a = p[3];
            p[0] = (unsigned(p[0]) * a) / 255;
            p[1] = (unsigned(p[1]) * a) / 255;
            p[2] = (unsigned(p[2]) * a) / 255;
        }

    Texture *layer = renderer->createTextureFromImageData(vec2(w,h), Texture::RGBA_32, data);
    STBI_FREE(data);
    return layer;
}

RENGINE_END_NAMESPACE
