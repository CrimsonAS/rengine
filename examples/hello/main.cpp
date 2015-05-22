#include "rengine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rengine;

// class Application
// {
//     virtual void initialize() = 0;
//     virtual void render() = 0;
// };

int main(int argc, char **argv)
{
    {
        int w, h, n;
        unsigned char *data = stbi_load("walker.png", &w, &h, &n, 4);
        printf("read image: %dx%d, %d components, topleft=%2x%2x%2x\n",
               w, h, n,
               data[0], data[1], data[2]);
    }


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