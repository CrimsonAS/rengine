#include "examples.h"



class BlurExample : public StandardSurfaceInterface
{
public:

    Node *update(Node *old) {
        if (old)
            return old;

        Layer *layer = rengine_loadImage(renderer(), "walker.png");

        vec2 size = surface()->size();
        float w2 = size.x / 2.0f;
        float h2 = size.y / 2.0f;
        float s2 = std::min(w2, h2);

        BlurNode *blurNode = BlurNode::create(2);

        AnimationClosure<BlurNode> *a = new AnimationClosure<BlurNode>(blurNode);
        a->setDuration(2);
        a->setDirection(Animation::Alternate);
        a->setIterations(-1);
        a->keyFrames.times() << 0 << 1;
        a->keyFrames.addValues<float, BlurNode_setRadius>() << 0 << 100;
        // animationManager()->startAnimation(a);

        mat4 matrix = mat4::translate2D(w2, h2)
                 * mat4::scale2D(s2, s2);
        return &(*TransformNode::create(matrix)
                 << &(*blurNode
                       << RectangleNode::create(rect2d::fromXywh(0.15, -0.5,  0.3, 0.3), vec4(1, 0, 0, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.15, -0.15, 0.3, 0.3), vec4(0, 1, 0, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.15,  0.2,  0.3, 0.3), vec4(0, 0, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh( 0.5, -0.5,  0.3, 0.3), vec4(1, 1, 0, 1))
                       << RectangleNode::create(rect2d::fromXywh( 0.5, -0.15, 0.3, 0.3), vec4(0, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh( 0.5,  0.2,  0.3, 0.3), vec4(1, 0, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.00, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.01, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.02, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.03, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.04, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.05, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.06, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.07, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.08, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.09, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.10, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.11, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.12, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.13, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.14, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.15, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.16, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.17, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.18, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.19, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(0.20, -0.1, 1/s2, 0.1), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.01, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.02, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.03, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.04, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.05, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.06, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.07, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.08, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.09, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.10, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.11, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.12, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.13, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.14, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.15, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.16, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.17, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.18, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.19, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << RectangleNode::create(rect2d::fromXywh(-0.1, 0.20, 0.1, 1/s2), vec4(1, 1, 1, 1))
                       << LayerNode::create(rect2d::fromXywh(-0.8, -0.5, 0.65, 1.0), layer)
                     )
                );
  }

};


RENGINE_MAIN(BlurExample)