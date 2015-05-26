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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;
using namespace std;

// Candidate for some util/.. thing, perhaps?
class SurfaceWithRenderer : public SurfaceInterface
{
public:

    SurfaceWithRenderer()
        : m_renderer(0)
    {
    }

    virtual Node *buildSceneGraph() = 0;

    void onRender() {
        if (!m_renderer) {
            m_renderer = Backend::get()->createRenderer();
            m_renderer->setTargetSurface(surface());
            m_renderer->initialize();
            m_renderer->setSceneRoot(buildSceneGraph());
        }

        m_renderer->render();
    }

    Renderer *renderer() const { return m_renderer; }

private:
    Renderer *m_renderer;
};

struct LayerNode_setSize {
    void operator()(const vec2 &v, LayerNode *l) { l->setSize(v); }
};

struct QuadTimingFunction {
    double operator()(double t) { return t * t; }
};

class Window : public SurfaceWithRenderer
{
public:
    Window()
    {
    }

    Node *buildSceneGraph() {
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

        LayerNode *layerNode = new LayerNode();
        layerNode->setLayer(layer);
        layerNode->setSize(layer->size());

        m_bounceAnimation.setTarget(layerNode);
        m_bounceAnimation.setIterations(-1);
        m_bounceAnimation.setDuration(2);

        m_keyFrames.addTime(0);
        m_keyFrames.addTime(1/3.);
        m_keyFrames.addTime(2/3.);
        m_keyFrames.addTime(1);

        m_keyFrameValues << vec2(w*2, h*2) << vec2(w*4, h*2) << vec2(w*2, h*4) << vec2(w*2,h*2);
        m_keyFrames.addValues(&m_keyFrameValues);

        m_bounceAnimation.setKeyFrames(&m_keyFrames);
        m_bounceAnimation.setRunning(true);

        root->append(layerNode);
        return root;
    }

    void onRender()
    {
        SurfaceWithRenderer::onRender();
        static double time = 0;
        m_bounceAnimation.tick(time);
        time += 0.0166;
        static int counter = 10 * 60;
        if (counter-- >= 0)
            surface()->requestRender();
        else
            exit(0);
    }

    Animation<LayerNode, QuadTimingFunction> m_bounceAnimation;
    KeyFrames<LayerNode> m_keyFrames;
    KeyFrameValues<vec2, LayerNode, LayerNode_setSize> m_keyFrameValues;
};

int main(int argc, char **argv)
{
    Backend *backend = Backend::get();

    Window window;
    Surface *surface = backend->createSurface(&window);
    surface->show();

    backend->run();

    return 0;
}