#include "test.h"

class ColorsAndPositions : public StaticRenderTest
{
public:
    Node *build() override {
        Node *root = Node::create();

        // White background to blend against..
        *root << RectangleNode::create(rect2d::fromXywh(18, 0, 6, 11), vec4(1, 1, 1, 1));

        for (int i=0; i<=10; ++i) {
            float c = i / 10.0;
            // black to primary and secondary colors
            *root << RectangleNode::create(rect2d::fromXywh( 0, i, 1, 1), vec4(c, 0, 0, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 1, i, 1, 1), vec4(0, c, 0, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 2, i, 1, 1), vec4(0, 0, c, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 3, i, 1, 1), vec4(c, c, 0, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 4, i, 1, 1), vec4(0, c, c, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 5, i, 1, 1), vec4(c, 0, c, 1));

            // primary and secondary colors to white
            *root << RectangleNode::create(rect2d::fromXywh( 6, i, 1, 1), vec4(1, c, c, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 7, i, 1, 1), vec4(c, 1, c, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 8, i, 1, 1), vec4(c, c, 1, 1));
            *root << RectangleNode::create(rect2d::fromXywh( 9, i, 1, 1), vec4(1, 1, c, 1));
            *root << RectangleNode::create(rect2d::fromXywh(10, i, 1, 1), vec4(c, 1, 1, 1));
            *root << RectangleNode::create(rect2d::fromXywh(11, i, 1, 1), vec4(1, c, 1, 1));

            // fade prim/sec colors on top of black
            *root << RectangleNode::create(rect2d::fromXywh(12, i, 1, 1), vec4(1, 0, 0, c));
            *root << RectangleNode::create(rect2d::fromXywh(13, i, 1, 1), vec4(0, 1, 0, c));
            *root << RectangleNode::create(rect2d::fromXywh(14, i, 1, 1), vec4(0, 0, 1, c));
            *root << RectangleNode::create(rect2d::fromXywh(15, i, 1, 1), vec4(1, 1, 0, c));
            *root << RectangleNode::create(rect2d::fromXywh(16, i, 1, 1), vec4(0, 1, 1, c));
            *root << RectangleNode::create(rect2d::fromXywh(17, i, 1, 1), vec4(1, 0, 1, c));

            // fade prim/sec colors on top of white (rect added before loop)
            *root << RectangleNode::create(rect2d::fromXywh(18, i, 1, 1), vec4(1, 0, 0, c));
            *root << RectangleNode::create(rect2d::fromXywh(19, i, 1, 1), vec4(0, 1, 0, c));
            *root << RectangleNode::create(rect2d::fromXywh(20, i, 1, 1), vec4(0, 0, 1, c));
            *root << RectangleNode::create(rect2d::fromXywh(21, i, 1, 1), vec4(1, 1, 0, c));
            *root << RectangleNode::create(rect2d::fromXywh(22, i, 1, 1), vec4(0, 1, 1, c));
            *root << RectangleNode::create(rect2d::fromXywh(23, i, 1, 1), vec4(1, 0, 1, c));
        }

        return root;
    }

    void check() override {
        for (int i=0; i<10; ++i) {
            float c = i / 10.0;

            // Black to primary and secondary
            check_pixel( 0, i, vec4(c, 0, 0, 1));
            check_pixel( 1, i, vec4(0, c, 0, 1));
            check_pixel( 2, i, vec4(0, 0, c, 1));
            check_pixel( 3, i, vec4(c, c, 0, 1));
            check_pixel( 4, i, vec4(0, c, c, 1));
            check_pixel( 5, i, vec4(c, 0, c, 1));

            // primary and secondary to white
            check_pixel( 6, i, vec4(1, c, c, 1));
            check_pixel( 7, i, vec4(c, 1, c, 1));
            check_pixel( 8, i, vec4(c, c, 1, 1));
            check_pixel( 9, i, vec4(1, 1, c, 1));
            check_pixel(10, i, vec4(c, 1, 1, 1));
            check_pixel(11, i, vec4(1, c, 1, 1));

            // alpha fade on top of black
            check_pixel(12, i, vec4(c, 0, 0, 1));
            check_pixel(13, i, vec4(0, c, 0, 1));
            check_pixel(14, i, vec4(0, 0, c, 1));
            check_pixel(15, i, vec4(c, c, 0, 1));
            check_pixel(16, i, vec4(0, c, c, 1));
            check_pixel(17, i, vec4(c, 0, c, 1));

            // alpha fade on top of white
            float ci = 1.0 - c;
            check_pixel(18, i, vec4( 1, ci, ci, 1));
            check_pixel(19, i, vec4(ci,  1, ci, 1));
            check_pixel(20, i, vec4(ci, ci,  1, 1));
            check_pixel(21, i, vec4( 1,  1, ci, 1));
            check_pixel(22, i, vec4(ci,  1,  1, 1));
            check_pixel(23, i, vec4( 1, ci,  1, 1));
        }
    }

    const char *name() const override { return "ColorsAndPositions"; }
};

class TexturesOnViewportEdge : public StaticRenderTest
{
public:
    Node *build() override {
        vec2 s = surface()->size();
        Node *root = Node::create();
        *root
            // Top-left, 50% red
            << &(*OpacityNode::create(0.5) << RectangleNode::create(rect2d::fromXywh(-1, -1, 2, 2), vec4(1, 0, 0, 1)))
            // Top-right, 25% green
            << &(*OpacityNode::create(0.25) << RectangleNode::create(rect2d::fromXywh(s.x-1, -1, 2, 2), vec4(0, 1, 0, 1)))
            // Bottom-left, 75% blue
            << &(*OpacityNode::create(0.75) << RectangleNode::create(rect2d::fromXywh(-1, s.y-1, 2, 2), vec4(0, 0, 1, 1)))
            // Bottom-right,
            << &(*OpacityNode::create(0.5) << RectangleNode::create(rect2d::fromXywh(s.x-1, s.y-1, 2, 2), vec4(1, 1, 1, 1)))
            ;

        return root;
    }

    void check() override {
        vec2 s = surface()->size();

        int r = s.x - 1;
        int b = s.y - 1;

        // Top/left 50% red
        check_pixel(0, 0, vec4(0.5, 0, 0, 1));
        check_pixel(1, 0, vec4(0.0, 0, 0, 1));
        check_pixel(1, 1, vec4(0.0, 0, 0, 1));
        check_pixel(0, 1, vec4(0.0, 0, 0, 1));

        // Top/right 25% green
        check_pixel(r,   0, vec4(0, 0.25, 0, 1));
        check_pixel(r-1, 0, vec4(0.0, 0, 0, 1));
        check_pixel(r,   1, vec4(0.0, 0, 0, 1));
        check_pixel(r-1, 1, vec4(0.0, 0, 0, 1));

        // Bottom/left 75% blue
        check_pixel(0, b,   vec4(0, 0, 0.75, 1));
        check_pixel(1, b,   vec4(0, 0, 0, 1));
        check_pixel(0, b-1, vec4(0, 0, 0, 1));
        check_pixel(1, b-1, vec4(0, 0, 0, 1));

        // Top/right 50% white
        check_pixel(r, b,  vec4(0.5, 0.5, 0.5, 1));
        check_pixel(r-1, b, vec4(0, 0, 0, 1));
        check_pixel(r, b-1, vec4(0, 0, 0, 1));
        check_pixel(r-1, b-1, vec4(0, 0, 0, 1));
    }

    const char *name() const override { return "TexturesOnViewportEdge"; }
};

class OpacityTextures : public StaticRenderTest
{
public:
    const char *name() const override { return "OpacityTextures"; }
    Node *build() override {
        Node *root = Node::create();

        *root

            // Two rectangles stacked on top of each other, no opacity bleed-through
            << &(*OpacityNode::create(0.5)
                 << RectangleNode::create(rect2d::fromXywh(10, 10, 10, 10), vec4(1, 0, 0, 1))
                 << RectangleNode::create(rect2d::fromXywh(11, 11, 10, 10), vec4(0, 0, 1, 1))
                )

            // Two rectangles stacked on top of each other, positioned with a transform node, no opacity bleed-through
            << &(*TransformNode::create(mat4::translate2D(30, 10))
                 << &(*OpacityNode::create(0.5)
                      << &(*TransformNode::create(mat4::translate2D(1, 1))
                           << RectangleNode::create(rect2d::fromXywh(0, 0, 10, 10), vec4(0, 1, 0, 1))
                          )
                      << &(*TransformNode::create(mat4::translate2D(2, 2))
                           << RectangleNode::create(rect2d::fromXywh(0, 0, 10, 10), vec4(1, 0, 0, 1))
                          )
                     )
                )

            // Nested layers
            << &(*OpacityNode::create(0.8)
                 << &(*OpacityNode::create(0.8)
                      << RectangleNode::create(rect2d::fromXywh(100, 10, 10, 10), vec4(1, 0, 0, 1))
                      << RectangleNode::create(rect2d::fromXywh(101, 11, 10, 10), vec4(0, 0, 1, 1))
                     )
                )

            ;

        return root;
    }

    void check() override {
        check_pixelsOutside(rect2d::fromXywh(10, 10, 11, 11), vec4(0, 0, 0, 1));
        check_pixel(10, 10, vec4(0.5, 0, 0, 1));
        check_pixel(11, 10, vec4(0.5, 0, 0, 1));
        check_pixel(10, 11, vec4(0.5, 0, 0, 1));
        check_pixel(11, 11, vec4(0, 0, 0.5, 1));
        check_pixel(11, 12, vec4(0, 0, 0.5, 1));
        check_pixel(12, 11, vec4(0, 0, 0.5, 1));

        check_pixelsOutside(rect2d::fromXywh(31, 11, 11, 11), vec4(0, 0, 0, 1));
        check_pixel(31, 11, vec4(0, 0.5, 0, 1));
        check_pixel(32, 11, vec4(0, 0.5, 0, 1));
        check_pixel(31, 12, vec4(0, 0.5, 0, 1));
        check_pixel(32, 12, vec4(0.5, 0, 0, 1));
        check_pixel(33, 12, vec4(0.5, 0, 0, 1));
        check_pixel(32, 13, vec4(0.5, 0, 0, 1));

        check_pixelsOutside(rect2d::fromXywh(100, 10, 11, 11), vec4(0, 0, 0, 1));
        check_pixel(100, 10, vec4(0.64, 0, 0, 1));
        check_pixel(101, 11, vec4(0, 0, 0.64, 1));
        check_pixel(110, 20, vec4(0, 0, 0.64, 1));
        check_pixel(110, 10, vec4(0, 0, 0, 1));
        check_pixel(100, 20, vec4(0, 0, 0, 1));
    }
};

int main(int argc, char *argv[])
{
    RENGINE_BACKEND backend;

    TestBase testBase;
// testBase.leaveRunning = true;
    testBase.addTest(new ColorsAndPositions());
    testBase.addTest(new TexturesOnViewportEdge());
    testBase.addTest(new OpacityTextures());
    testBase.show();

    backend.run();

    return 0;
}
