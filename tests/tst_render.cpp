#include "test.h"

class ColorsAndPositions : public StaticRenderTest
{
public:
    Node *build() override {
        Node *root = new Node();

        // White background to blend against..
        *root << new RectangleNode(rect2d::fromXywh(18, 0, 6, 11), vec4(1, 1, 1, 1));

        for (int i=0; i<=10; ++i) {
            float c = i / 10.0;
            // black to primary and secondary colors
            *root << new RectangleNode(rect2d::fromXywh( 0, i, 1, 1), vec4(c, 0, 0, 1));
            *root << new RectangleNode(rect2d::fromXywh( 1, i, 1, 1), vec4(0, c, 0, 1));
            *root << new RectangleNode(rect2d::fromXywh( 2, i, 1, 1), vec4(0, 0, c, 1));
            *root << new RectangleNode(rect2d::fromXywh( 3, i, 1, 1), vec4(c, c, 0, 1));
            *root << new RectangleNode(rect2d::fromXywh( 4, i, 1, 1), vec4(0, c, c, 1));
            *root << new RectangleNode(rect2d::fromXywh( 5, i, 1, 1), vec4(c, 0, c, 1));

            // primary and secondary colors to white
            *root << new RectangleNode(rect2d::fromXywh( 6, i, 1, 1), vec4(1, c, c, 1));
            *root << new RectangleNode(rect2d::fromXywh( 7, i, 1, 1), vec4(c, 1, c, 1));
            *root << new RectangleNode(rect2d::fromXywh( 8, i, 1, 1), vec4(c, c, 1, 1));
            *root << new RectangleNode(rect2d::fromXywh( 9, i, 1, 1), vec4(1, 1, c, 1));
            *root << new RectangleNode(rect2d::fromXywh(10, i, 1, 1), vec4(c, 1, 1, 1));
            *root << new RectangleNode(rect2d::fromXywh(11, i, 1, 1), vec4(1, c, 1, 1));

            // fade prim/sec colors on top of black
            *root << new RectangleNode(rect2d::fromXywh(12, i, 1, 1), vec4(1, 0, 0, c));
            *root << new RectangleNode(rect2d::fromXywh(13, i, 1, 1), vec4(0, 1, 0, c));
            *root << new RectangleNode(rect2d::fromXywh(14, i, 1, 1), vec4(0, 0, 1, c));
            *root << new RectangleNode(rect2d::fromXywh(15, i, 1, 1), vec4(1, 1, 0, c));
            *root << new RectangleNode(rect2d::fromXywh(16, i, 1, 1), vec4(0, 1, 1, c));
            *root << new RectangleNode(rect2d::fromXywh(17, i, 1, 1), vec4(1, 0, 1, c));

            // fade prim/sec colors on top of white (rect added before loop)
            *root << new RectangleNode(rect2d::fromXywh(18, i, 1, 1), vec4(1, 0, 0, c));
            *root << new RectangleNode(rect2d::fromXywh(19, i, 1, 1), vec4(0, 1, 0, c));
            *root << new RectangleNode(rect2d::fromXywh(20, i, 1, 1), vec4(0, 0, 1, c));
            *root << new RectangleNode(rect2d::fromXywh(21, i, 1, 1), vec4(1, 1, 0, c));
            *root << new RectangleNode(rect2d::fromXywh(22, i, 1, 1), vec4(0, 1, 1, c));
            *root << new RectangleNode(rect2d::fromXywh(23, i, 1, 1), vec4(1, 0, 1, c));
        }

        return root;
    }

    bool check() override {
        for (int i=0; i<10; ++i) {
            float c = i / 10.0;

            // Black to primary and secondary
            checkPixel( 0, i, vec4(c, 0, 0, 1));
            checkPixel( 1, i, vec4(0, c, 0, 1));
            checkPixel( 2, i, vec4(0, 0, c, 1));
            checkPixel( 3, i, vec4(c, c, 0, 1));
            checkPixel( 4, i, vec4(0, c, c, 1));
            checkPixel( 5, i, vec4(c, 0, c, 1));

            // primary and secondary to white
            checkPixel( 6, i, vec4(1, c, c, 1));
            checkPixel( 7, i, vec4(c, 1, c, 1));
            checkPixel( 8, i, vec4(c, c, 1, 1));
            checkPixel( 9, i, vec4(1, 1, c, 1));
            checkPixel(10, i, vec4(c, 1, 1, 1));
            checkPixel(11, i, vec4(1, c, 1, 1));

            // alpha fade on top of black
            checkPixel(12, i, vec4(c, 0, 0, 1));
            checkPixel(13, i, vec4(0, c, 0, 1));
            checkPixel(14, i, vec4(0, 0, c, 1));
            checkPixel(15, i, vec4(c, c, 0, 1));
            checkPixel(16, i, vec4(0, c, c, 1));
            checkPixel(17, i, vec4(c, 0, c, 1));

            // alpha fade on top of white
            float ci = 1.0 - c;
            checkPixel(18, i, vec4( 1, ci, ci, 1));
            checkPixel(19, i, vec4(ci,  1, ci, 1));
            checkPixel(20, i, vec4(ci, ci,  1, 1));
            checkPixel(21, i, vec4( 1,  1, ci, 1));
            checkPixel(22, i, vec4(ci,  1,  1, 1));
            checkPixel(23, i, vec4( 1, ci,  1, 1));



        }
        return true;
    }

    const char *name() const override { return "ColorsAndPositions"; }
};

class PositionsOfLayers : public StaticRenderTest
{
public:
    Node *build() override {
        vec2 size = surface()->size();
        Node *root = new Node();
        *root
            << &(*new OpacityNode(0.9) << new RectangleNode(rect2d::fromXywh(10, 10, 1, 1), vec4(1, 0, 1, 1)))

            // Top-left, 50% red
            << &(*new OpacityNode(0.5) << new RectangleNode(rect2d::fromXywh(-1, -1, 2, 2), vec4(1, 0, 0, 1)))

            // Top-right, 25% green
            << &(*new OpacityNode(0.25) << new RectangleNode(rect2d::fromXywh(size.x-1, -1, 2, 2), vec4(0, 1, 0, 1)))

            // Bottom-left, 75% blue
            << &(*new OpacityNode(0.75) << new RectangleNode(rect2d::fromXywh(-1, size.y-10, 2, 2), vec4(0, 0, 1, 1)))

            // Bottom-right,
            << &(*new OpacityNode(0.5) << new RectangleNode(rect2d::fromXywh(size.x-1, size.y-1, 2, 2), vec4(1, 1, 1, 1)))
            ;

        return root;
    }

    bool check() {
        return true;
    }

    const char *name() const override { return "PositionsOfLayers"; }
};

int main(int argc, char *argv[])
{
    TestBase testBase;
    // testBase.leaveRunning = true;
    testBase.addTest(new ColorsAndPositions());
    testBase.addTest(new PositionsOfLayers());

    std::unique_ptr<Backend> backend(Backend::get());
    std::unique_ptr<Surface>  surface(backend->createSurface(&testBase));
    surface->show();
    surface->requestRender();

    backend->run();

    return 0;
}