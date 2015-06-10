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

#include "rengine.h"

#include <iostream>
#include <list>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;
using namespace std;
using namespace std::chrono;





// Candidate for some util/.. thing, perhaps?
struct LayerNode_setSize {
    void operator()(const vec2 &v, LayerNode *l) { l->setSize(v); }
};

struct QuadTimingFunction {
    double operator()(double t) { return t * t; }
};

class MyWindow : public StandardSurfaceInterface
{
public:
    MyWindow()
    {
    }

    Node *update(Node *oldRoot) {
        if (oldRoot)
            return oldRoot;

        // read the image...
        int w, h, n;
        unsigned char *data = stbi_load("../examples/images/walker.png", &w, &h, &n, 4);
        if (!data) {
            data = stbi_load("examples", &w, &h, &n, 4);
        }
        if (!data) {
            cout << "Failed to find the image 'walker.png' under 'examples/images' or '../examples/images'. "
                 << "We're a bit dumb, you see, and can't find images unless you've built directly in the "
                 << "source directory or in a build/debug/release subdirectory..." << endl;
            exit(1);
        }

        Layer *layer = renderer()->createLayerFromImageData(vec2(w,h), Layer::RGBA_32, data);
        STBI_FREE(data);

        Node *root = new Node();

        TransformNode *xnode = new TransformNode();
        xnode->setMatrix(mat4::translate2D(200, 100));

        m_layerNode = new LayerNode();
        m_layerNode->setLayer(layer);
        m_layerNode->setSize(layer->size());

        m_bounceAnimation.setIterations(-1);
        m_bounceAnimation.setDuration(1);
        m_bounceAnimation.setDirection(Animation::Alternate);
        m_bounceAnimation.setRunning(true);

        m_keyFrames.times() << 0 << 1;
        m_keyFrames.addValues<vec2, LayerNode_setSize>() << vec2(w*2, h) << vec2(w, h*2);


        root->append(xnode);
        xnode->append(m_layerNode);

        return root;
    }

    void onRender()
    {
        StandardSurfaceInterface::onRender();
        static double time = 0;
        m_bounceAnimation.tick(time, m_layerNode, &m_keyFrames);
        time += 0.0166;
        static int counter = 10 * 60;
        if (counter-- >= 0)
            surface()->requestRender();
        else
            exit(0);
    }

    Animation m_bounceAnimation;
    KeyFrames<LayerNode> m_keyFrames;
    LayerNode *m_layerNode;
};

RENGINE_MAIN(MyWindow)

// int main(int argc, char **argv)
// {
//     Backend *backend = Backend::get();

//     Window window;
//     Surface *surface = backend->createSurface(&window);
//     surface->show();

//     backend->run();

//     return 0;
// }