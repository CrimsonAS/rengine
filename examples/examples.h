#pragma once

#include "rengine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;
using namespace std;

RENGINE_BEGIN_NAMESPACE

inline Texture *rengine_loadImage(Renderer *renderer, const char *file)
{
    // read the image...
    int w, h, n;
    char location[1024];
    sprintf(location, "../examples/images/%s", file);
    unsigned char *data = stbi_load(location, &w, &h, &n, 4);
    if (!data) {
        sprintf(location, "examples/images/%s", file);
        data = stbi_load(file, &w, &h, &n, 4);
    }
    if (!data) {
        cout << "Failed to find the image 'walker.png' under 'examples/images' or '../examples/images'. "
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
    assert(layer);
    STBI_FREE(data);
    return layer;
}

RENGINE_END_NAMESPACE