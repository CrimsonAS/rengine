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

        rect2d geometry(0, 0, s.x, s.y);
        vec4 color(1, 0, 0, 0.1);
        RectangleNode *root = RectangleNode::create(geometry, color);

        for (int i=1; i<nodeCount; ++i) {
            RectangleNode *child = RectangleNode::create(geometry, color);
            *root << child;
        }

        return root;
    }
};

RENGINE_DEFINE_BACKEND
RENGINE_DEFINE_NODE_ALLOCATION_POOLS

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
