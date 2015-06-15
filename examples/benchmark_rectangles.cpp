#include "rengine.h"
#include "examples.h"

class Rectangles : public StandardSurfaceInterface
{
public:
    Node *update(Node *root) {
        if (root)
            delete root;

        root = new Node();

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
            *root << new RectangleNode(rect2d::fromXywh(rand() % w, rand() % h, rw, rh), color);
        }

        surface()->requestRender();

        return root;
    }
};

RENGINE_MAIN(Rectangles)