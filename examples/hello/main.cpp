#include "rengine.h"

using namespace rengine;

int main(int argc, char **argv)
{
    System *system = System::get();

    Window *window = system->createWindow();
    Renderer *renderer = system->createRenderer(window);

    // build a simple scene graph...
    Node *root = new Node();
    // add a few notes...

    renderer->setSceneRoot(root);
    renderer->render();

    window->show();

    system->run();

    // while (true) {
        // tick animations
        // process events
        // update scene
        // draw and present scene
    // }

}