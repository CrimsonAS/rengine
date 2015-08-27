#include "rengine.h"
#include "examples.h"

RENGINE_ALLOCATION_POOL_NODE_DEFINITIONS;

class Rectangles : public StandardSurfaceInterface
{
public:
    Node *update(Node *root) {
        if (root)
            root->destroy();

        root = Node::create();

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
            RectangleNode *rect = RectangleNode::create();
            rect->setGeometry(rect2d::fromXywh(rand() % w, rand() % h, rw, rh));
            rect->setColor(color);
            *root << rect;
        }

        surface()->requestRender();

        return root;
    }
};


int main(int argc, char **argv) {
    RENGINE_ALLOCATION_POOL(RectangleNode, 1024);
    RENGINE_ALLOCATION_POOL(Node, 64);
    rengine_main<Rectangles>(argc, argv);
    return 0;
}
