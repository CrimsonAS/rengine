#include "test.h"

static bool alreadyDone = false;

class RedGreenBlue : public StandardSurfaceInterface
{
public:
    Node *update(Node *root) override {
        if (root)
            return 0;

        root = new Node();

        *root << new RectangleNode(rect2d::fromXywh(0, 0, 1, 1), vec4(1, 0, 0, 1))
              << new RectangleNode(rect2d::fromXywh(1, 0, 1, 1), vec4(0, 1, 0, 1))
              << new RectangleNode(rect2d::fromXywh(2, 0, 1, 1), vec4(0, 0, 1, 1));

        return root;
    }

    void afterRender() override {
        vec2 size = surface()->size();
        unsigned *pixels = (unsigned *) malloc(size.x * size.y * sizeof(unsigned));

        bool ok = renderer()->readPixels(0, 0, size.x, size.y, (unsigned char *) pixels);
        check_true(ok);

        int offset = (size.y - 1) * size.x;

        check_equal_hex(pixels[0 + offset], 0xff0000ff);
        check_equal_hex(pixels[1 + offset], 0xff00ff00);
        check_equal_hex(pixels[2 + offset], 0xffff0000);

        cout << "tst_render: RedGreenBlue: ok" << endl;

        alreadyDone = true;
        Backend::get()->quit();
    }
};

int main(int argc, char *argv[])
{
    std::unique_ptr<Backend> backend(Backend::get());
    RedGreenBlue iface;
    Surface *surface = backend->createSurface(&iface);
    surface->show();
    if (!alreadyDone)
        backend->run();
    return 0;
}