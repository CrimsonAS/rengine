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

RENGINE_NODE_DEFINE_ALLOCATION_POOLS;
RENGINE_NODE_DEFINE_SIGNALS
RENGINE_BACKEND_DEFINE

class Rectangles : public StandardSurfaceInterface
{
public:
    Node *update(Node *root) {
        if (root)
            root->destroy();

        root = Node::create();

        vec2 size = surface()->size();

        int rw = 100;
        int rh = 20;
        int w = size.x - rw;
        int h = size.y - rh;

        int count = 1000;

        static int c = 1;
        c = (c + 1) % 2;
        for (int i=0; i<count; ++i) {
            vec4 color((rand() % 100)/100.0,
                       (rand() % 100)/100.0,
                       (rand() % 100)/100.0,
                       0.9);
            RectangleNode *rect = RectangleNode::create();
            rect->setGeometry(rect2d::fromXywh(rand() % w, rand() % h, rw, rh));
            rect->setColor(color);
            *root << rect;
        }

        surface()->requestRender();

        return root;
    }
};


int main(int argc, char **argv) {
    RENGINE_ALLOCATION_POOL(RectangleNode, rengine_RectangleNode, 1024);
    RENGINE_ALLOCATION_POOL(Node, rengine_Node, 64);
    rengine_main<Rectangles>(argc, argv);
    return 0;
}
