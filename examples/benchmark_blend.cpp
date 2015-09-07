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

static int nodeCount = 4;

using namespace std::chrono;

static steady_clock::time_point then;

class BlendBenchWindow : public StandardSurfaceInterface
{
public:

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

        cout << "creating " << nodeCount << " layers.." << endl;

        vec4 color((rand() % 100) / 100.0,
                   (rand() % 100) / 100.0,
                   (rand() % 100) / 100.0,
                   0.9);
        rect2d geometry(0, 0, s.x, s.y);
        RectangleNode *root = RectangleNode::create(geometry, color);

        float bw = std::max(s.x, s.y) * 0.4;
        TransformNode *rotation = TransformNode::create(mat4::rotate2D(1));
        *root << &(*TransformNode::create(mat4::translate2D(s.x / 2, s.y / 2))
                   << &(*rotation
                        << RectangleNode::create(rect2d::fromXywh(-bw/2.0, -5, bw, 10), vec4(0, 0, 1, 1))
                       )
                  );

        for (int i=1; i<nodeCount; ++i) {
            geometry = rect2d(i * 4, i * 4, s.x, s.y);
            color = vec4(1.0,
                         1.0,
                         1.0,
                         0.1);
            RectangleNode *child = RectangleNode::create(geometry, color);
            *root << child;
        }


        AnimationClosure<TransformNode, LinearTimingFunction> *anim
            = new AnimationClosure<TransformNode, LinearTimingFunction>(rotation);
        anim->setDuration(5);
        anim->setIterations(-1);
        anim->keyFrames.times() << 0 << 1;
        anim->keyFrames.addValues<double, TransformNode_rotateAroundZ>() << 0 << M_PI * 2.0;
        animationManager()->startAnimation(anim);

        return root;
    }
};

RENGINE_NODE_DEFINE_ALLOCATION_POOLS
RENGINE_NODE_DEFINE_SIGNALS
RENGINE_BACKEND_DEFINE

int main(int argc, char **argv) {

    for (int i=0; i<argc; ++i) {
        if (i + 1 < argc && std::string(argv[i]) == "--count") {
            nodeCount = atoi(argv[++i]);
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
