#pragma once

#include "rengine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;
using namespace std;

RENGINE_BEGIN_NAMESPACE

Layer *rengine_loadImage(Renderer *renderer, const char *file)
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

    Layer *layer = renderer->createLayerFromImageData(vec2(w,h), Layer::RGBA_32, data);
    assert(layer);
    STBI_FREE(data);
    return layer;
}

RENGINE_END_NAMESPACE