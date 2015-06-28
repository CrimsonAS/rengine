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

using namespace rengine;
using namespace std;

class MyWindow : public StandardSurfaceInterface
{
public:

    Node *createSubtree(float dx, float dy) {
        return
            &(*new TransformNode(mat4::translate2D(dx, dy))
              << &(*new ColorFilterNode()
                   << new RectangleNode(rect2d::fromXywh(  0, 0, 90, 90), vec4(1, 0, 0, 1))
                   << new RectangleNode(rect2d::fromXywh(100, 0, 90, 90), vec4(1, 1, 0, 1))
                   << new RectangleNode(rect2d::fromXywh(200, 0, 90, 90), vec4(0, 1, 0, 1))
                   << new RectangleNode(rect2d::fromXywh(300, 0, 90, 90), vec4(0, 1, 1, 1))
                   << new RectangleNode(rect2d::fromXywh(400, 0, 90, 90), vec4(0, 0, 1, 1))
                   << new RectangleNode(rect2d::fromXywh(500, 0, 90, 90), vec4(1, 0, 1, 1))
                  )
             );
    }

    Node *update(Node *old) {
        if (old)
            return old;

        Node *root = new Node();

        float pos = 100;

        Node *unfiltered = createSubtree(100, 100);
        root->append(unfiltered);

        { // desaturate
            pos += 100;
            Node *tree = createSubtree(100, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->children().at(0));
            node->setColorMatrix(colorMatrix_saturation(0.2));
            root->append(tree);
        }

        { // grayscale
            pos += 100;
            Node *tree = createSubtree(100, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->children().at(0));
            node->setColorMatrix(colorMatrix_grayscale());
            root->append(tree);
        }

        { // brightness
            pos += 100;
            Node *tree = createSubtree(100, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->children().at(0));
            node->setColorMatrix(colorMatrix_brightness(0.3));
            root->append(tree);
        }

        { // hue shift
            pos += 100;
            Node *tree = createSubtree(100, pos);
            ColorFilterNode *node = static_cast<ColorFilterNode *>(tree->children().at(0));
            node->setColorMatrix(colorMatrix_hue(1.0));
            root->append(tree);
        }




        return root;
    }
};

RENGINE_MAIN(MyWindow)