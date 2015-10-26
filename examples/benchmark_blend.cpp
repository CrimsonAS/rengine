/*
    Copyright (c) 2015, Gunnar Sletta <gunnar@sletta.org>
    Copyright (c) 2015, Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
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

#include "rengine.h"
#include "examples.h"

static int nodeCount = 4;
static bool useTextures = false;

using namespace std::chrono;

static steady_clock::time_point then;

class BlendBenchWindow : public StandardSurfaceInterface
{
public:

    float rnd() { return (rand() % 100) / 100.0; }

    Node *update(Node *old) {

        surface()->requestRender();

        static int frameCounter = 0;
        ++frameCounter;

        steady_clock::time_point now = steady_clock::now();

        if (now > then + std::chrono::milliseconds(int(1000))) {
            double delta = duration<double>(now - then).count();
            cout << "FPS: " << (frameCounter / delta) << endl;
            frameCounter = 0;
            then = now;
        }

        if (old)
            return old;

        vec2 s = surface()->size();
        vec2 s2 = s / 2.0f;
        float dim = std::max(s.x, s.y) * 0.9;
        float dim2 = dim / 2.0f;
        rect2d geometry(-dim2, -dim2, dim, dim);

        if (useTextures)
            cout << "creating " << nodeCount << " texture layers.." << endl;
        else
            cout << "creating " << nodeCount << " solid-color layers.." << endl;

        Node *root = Node::create();
        for (int i=0; i<nodeCount; ++i) {
            TransformNode *rotation = TransformNode::create();

            *root << &(*TransformNode::create(mat4::translate2D(s2.x, s2.y))
                       << rotation
                      );
            if (useTextures) {
                cout << "Creating texture: " << (i+1) << endl;
                Texture *texture = rengine_fractalTexture(renderer(), vec2(dim, dim));
                *rotation << TextureNode::create(geometry, texture);
            } else {
                *rotation << RectangleNode::create(geometry, vec4(rnd(), rnd(), rnd(), 0.5));
            }
            animation_rotateZ(animationManager(), rotation, 4 + i);
        }

        return root;
    }
};

RENGINE_NODE_DEFINE_ALLOCATION_POOLS
RENGINE_NODE_DEFINE_SIGNALS
RENGINE_BACKEND_DEFINE

int main(int argc, char **argv) {

    for (int i=0; i<argc; ++i) {
        std::string arg(argv[i]);
        if (i + 1 < argc && arg == "--count") {
            nodeCount = atoi(argv[++i]);
        } else if (arg == "--textures") {
            useTextures = true;
        } else if (arg == "-h" || arg == "--help") {
            cout << "Usage: " << endl
                 << " > " << argv[0] << " [options]" << endl
                 << endl
                 << "Options:" << endl
                 << "  --count [x]      Number of layers" << endl
                 << "  --textures       Use textures rather than solid fills" << endl;
        }
    }

    std::unique_ptr<Backend> backend(Backend::get());

    BlendBenchWindow iface;

    then = steady_clock::now();

    Surface *surface = backend->createSurface(&iface);
    surface->show();

    backend->run();

    return 0;
}
