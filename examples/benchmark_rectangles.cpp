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
*/

#include "rengine.h"
#include "examples.h"

#define  STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

RENGINE_DEFINE_GLOBALS

static int nodeCount = 1000;
static bool interleaved = false;
static bool textured = false;
static std::vector<Texture *> texturePool;

class Rectangles : public StandardSurface
{
public:
    Rectangles()
    {
    }

    Node *update(Node *root) override {

        // if --textures, we have nodeCount textures (all the nodes)
        // if --interleaved, we have half of them.
        const unsigned int textureCount = interleaved ? nodeCount / 2 : (textured ? nodeCount : 0);

        if (texturePool.size() != textureCount) {
            for (unsigned int i=0; i<textureCount; ++i) {
                texturePool.push_back(rengine_fractalTexture(renderer(), vec2(100, 20)));
            }
        }

        if (root)
            root->destroy();

        rengine_countFps();

        root = Node::create();

        vec2 s = size();

        int rw = 100;
        int rh = 20;
        int w = s.x - rw;
        int h = s.y - rh;

        int count = nodeCount;

        for (int i=0; i<count; ++i) {

            rect2d geometry = rect2d::fromXywh(rand() % w, rand() % h, rw, rh);

            if (textured || (interleaved && (i % 2) == 1)) {
                TextureNode *tn = TextureNode::create();
                tn->setGeometry(geometry);
                tn->setTexture(texturePool[i / 2]);
                *root << tn;
            } else {
                vec4 color((rand() % 100)/100.0,
                           (rand() % 100)/100.0,
                           (rand() % 100)/100.0,
                           0.9);
                RectangleNode *rect = RectangleNode::create();
                rect->setGeometry(geometry);
                rect->setColor(color);
                *root << rect;
            }
        }

        requestRender();

        return root;
    }
};


int main(int argc, char **argv) {

    for (int i=0; i<argc; ++i) {
        std::string arg(argv[i]);
        if (i + 1 < argc && arg == "--count") {
            nodeCount = atoi(argv[++i]);
        } else if (i < argc && arg == "--interleaved") {
            interleaved = true;
        } else if (i < argc && arg == "--textured") {
            textured = true;
        }
    }

    if (interleaved)
        textured = false;

    std::cout << "Using " << nodeCount << " nodes..." << std::endl;
    std::cout << "  --interleaved ....: " << (interleaved ? "yes" : "no") << std::endl;
    std::cout << "  --textured .......: " << (textured ? "yes" : "no") << std::endl;

    RENGINE_ALLOCATION_POOL(RectangleNode, rengine_RectangleNode, 1024);
    RENGINE_ALLOCATION_POOL(TextureNode, rengine_TextureNode, 1024);
    RENGINE_ALLOCATION_POOL(Node, rengine_Node, 64);
    rengine_main<Rectangles>(argc, argv);
    return 0;
}
