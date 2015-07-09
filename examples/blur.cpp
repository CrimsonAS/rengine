#include "examples.h"



class BlurExample : public StandardSurfaceInterface
{
public:

    Node *update(Node *old) {
        if (old)
            return old;

        Texture *layer = rengine_loadImage(renderer(), "walker.png");

        vec2 size = surface()->size();
        float w2 = size.x / 2.0f;
        float h2 = size.y / 2.0f;
        float s2 = std::min(w2, h2);

        BlurNode *blurNode = BlurNode::create(37);

        AnimationClosure<BlurNode> *a = new AnimationClosure<BlurNode>(blurNode);
        a->setDuration(2);
        a->setDirection(Animation::Alternate);
        a->setIterations(-1);
        a->keyFrames.times() << 0 << 1;
        a->keyFrames.addValues<float, BlurNode_setRadius>() << 0 << 100;
        animationManager()->startAnimation(a);

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
                       << TextureNode::create(rect2d::fromXywh(-0.8, -0.5, 0.65, 1.0), layer)
                     )
                );
  }

};


RENGINE_MAIN(BlurExample)