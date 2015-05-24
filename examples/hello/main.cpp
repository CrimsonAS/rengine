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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;

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
            m_renderer = System::get()->createRenderer();
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
        assert(data);
        printf("read image: %dx%d, %d components\n",
               w, h, n);
        assert(data);

        Layer *layer = renderer()->createLayerFromImageData(vec2(w,h), Layer::RGBA_32, data);
        STBI_FREE(data);

        Node *root = new Node();
        int count = 10;

        for (int i=0; i<count; ++i) {
            LayerNode *layerNode = new LayerNode();
            layerNode->setLayer(layer);
            layerNode->setSize(layer->size());

            float s = 1 + i * 0.1;
            TransformNode *xnode = new TransformNode();
            xnode->setMatrix(mat4::translate2D(200 + i * 100, 100 + i * 10)
                             * mat4::scale2D(s, s)
                             * mat4::rotate2D(i * 0.05));
            xnode->append(layerNode);
            root->append(xnode);
        }
        return root;
    }
};

int main(int argc, char **argv)
{
    System *system = System::get();

    Window window;
    Surface *surface = system->createSurface(&window);
    surface->show();

    system->run();

    return 0;
}