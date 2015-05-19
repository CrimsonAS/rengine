#include "rengine.h"

using namespace rengine;

int main(int argc, char **argv)
{
    System *system = System::get();

    Surface *surface = system->createSurface();
    surface->show();

    Renderer *renderer = system->createRenderer();

    // build a simple scene graph...
    Node *root = new Node();
    // add a few nodes...

    if (renderer) {
        renderer->setTargetSurface(surface);
        renderer->setSceneRoot(root);
    } else {
        printf("System::createRenderer() returned 0, don't expect too much...\n");
        return 0;
    }

    // system->run();

    int frames = 190;
    while (--frames) {
        system->processEvents();
        renderer->render();

        // tick animations
        // process events
        // update scene
        // draw and present scene
    }

    return 0;
}